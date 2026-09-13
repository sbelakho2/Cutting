#include "serial_comm.hpp"
#include <algorithm>
#include <cstring>

namespace starz::hmi {

SerialComm::SerialComm(QObject* parent)
    : QObject(parent), serial_(new QSerialPort(this)), heartbeat_timer_(new QTimer(this)),
      freshness_timer_(new QTimer(this)) {
    clock_.start();
    connect(serial_, &QSerialPort::readyRead, this, &SerialComm::on_ready_read);
    connect(heartbeat_timer_, &QTimer::timeout, this, &SerialComm::on_heartbeat_timeout);
    connect(freshness_timer_, &QTimer::timeout, this, &SerialComm::on_freshness_timeout);
    connect(serial_, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::ResourceError || error == QSerialPort::ReadError || error == QSerialPort::WriteError) {
            const auto message = serial_->errorString();
            disconnect_port();
            emit packet_error(message);
        }
    });
    heartbeat_timer_->setInterval(250); // Deadline remains 750 ms, not 1.5 s.
    freshness_timer_->setInterval(50);
    serial_->setReadBufferSize(4 * static_cast<qint64>(protocol::MAX_FRAME_SIZE));
}

SerialComm::~SerialComm() {
    // The owning window's layout may already be destroyed during QObject child
    // cleanup. Never emit UI callbacks from a child destructor.
    blockSignals(true);
    serial_->blockSignals(true);
    disconnect_port();
}

uint64_t SerialComm::now_ms() const {
    return static_cast<uint64_t>(std::max<qint64>(clock_.elapsed(), 0));
}

bool SerialComm::connect_to_port(const QString& port_name, qint32 baud_rate) {
    disconnect_port();
    serial_->setPortName(port_name);
    serial_->setBaudRate(baud_rate);
    serial_->setDataBits(QSerialPort::Data8);
    serial_->setParity(QSerialPort::NoParity);
    serial_->setStopBits(QSerialPort::OneStop);
    serial_->setFlowControl(QSerialPort::NoFlowControl);
    if (!serial_->open(QIODevice::ReadWrite)) {
        emit connection_status_changed(false, QString("Failed to open %1: %2").arg(port_name, serial_->errorString()));
        return false;
    }
    telemetry_status_.set_connected(true);
    parser_.reset();
    tx_seq_ = 0;
    heartbeat_timer_->start();
    freshness_timer_->start();
    emit connection_status_changed(true, QString("Reference link %1 @ %2 baud; waiting for valid telemetry").arg(port_name).arg(baud_rate));
    send_ping();
    return true;
}

void SerialComm::invalidate_telemetry() {
    telemetry_status_.invalidate();
    last_fresh_ = false;
    emit telemetry_validity_changed(false);
}

void SerialComm::disconnect_port() {
    heartbeat_timer_->stop();
    freshness_timer_->stop();
    parser_.reset();
    telemetry_status_.set_connected(false);
    invalidate_telemetry();
    if (serial_->isOpen()) serial_->close();
    emit connection_status_changed(false, "Disconnected; telemetry unknown");
}

bool SerialComm::is_connected() const { return serial_->isOpen(); }

bool SerialComm::has_fresh_telemetry() const {
    return is_connected() && telemetry_status_.fresh(now_ms());
}

void SerialComm::on_ready_read() {
    on_freshness_timeout(); // Expire a truncated frame even if the GUI timer was delayed.
    const auto data = serial_->readAll();
    for (char byte : data) parse_byte(static_cast<uint8_t>(byte));
}

void SerialComm::on_heartbeat_timeout() {
    if (is_connected()) send_ping();
}

void SerialComm::on_freshness_timeout() {
    const auto now = now_ms();
    if (parser_.partial() && now - frame_started_ms_ >= protocol::FRAME_TIMEOUT_US / 1000) {
        parser_.reset();
        invalidate_telemetry();
        emit packet_error("Truncated reference frame expired");
    }
    const bool fresh = has_fresh_telemetry();
    if (!fresh && last_fresh_) invalidate_telemetry();
}

void SerialComm::send_packet(protocol::MsgId msg_id, const uint8_t* payload, uint8_t len) {
    if (len > protocol::MAX_PAYLOAD_SIZE || (len != 0 && payload == nullptr)) {
        emit packet_error("Invalid transmit payload length");
        return;
    }
    // Read-only reference monitor. Even valid telemetry cannot commission this
    // application. Abort remains available without telemetry; it is NOT an E-stop.
    if (msg_id != protocol::MsgId::CMD_PING && msg_id != protocol::MsgId::CMD_JOB_ABORT) {
        emit packet_error(has_fresh_telemetry()
            ? "Command disabled: reference prototype NOT FOR MACHINE OPERATION"
            : "Command denied: no fresh valid reference telemetry");
        return;
    }
    if (!is_connected()) return;
    if (serial_->bytesToWrite() > 4 * static_cast<qint64>(protocol::MAX_FRAME_SIZE)) {
        disconnect_port();
        emit packet_error("Reference transmit queue stalled; link closed");
        return;
    }
    protocol::Packet packet;
    packet.msg_id = static_cast<uint8_t>(msg_id);
    packet.seq = tx_seq_++;
    packet.len = len;
    if (len > 0) std::memcpy(packet.payload.data(), payload, len);
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> buffer{};
    const auto length = packet.serialize(buffer.data(), buffer.size());
    if (length == 0 || serial_->write(reinterpret_cast<const char*>(buffer.data()), static_cast<qint64>(length)) != static_cast<qint64>(length)) {
        disconnect_port();
        emit packet_error("Reference frame write failed");
    }
}

void SerialComm::send_ping() { send_packet(protocol::MsgId::CMD_PING, nullptr, 0); }
void SerialComm::send_job_pause() { send_packet(protocol::MsgId::CMD_JOB_PAUSE, nullptr, 0); }
void SerialComm::send_job_resume() { send_packet(protocol::MsgId::CMD_JOB_RESUME, nullptr, 0); }
void SerialComm::send_job_abort() { send_packet(protocol::MsgId::CMD_JOB_ABORT, nullptr, 0); }
void SerialComm::send_home() { send_packet(protocol::MsgId::CMD_HOME_ALL, nullptr, 0); }
void SerialComm::send_solenoid_door() { send_packet(protocol::MsgId::CMD_SOLENOID_DOOR, nullptr, 0); }

void SerialComm::send_job_start(int32_t length_um, int32_t lead_um, int32_t tail_um, int32_t depth_um, uint32_t quantity) {
    const protocol::JobStartPayload job{length_um, lead_um, tail_um, depth_um, quantity};
    if (protocol::validate_job(job) != protocol::CommandResult::Accepted) {
        emit packet_error("Invalid job parameters");
        return;
    }
    const auto payload = protocol::encode_job(job);
    send_packet(protocol::MsgId::CMD_JOB_START, payload.data(), protocol::JOB_PAYLOAD_SIZE);
}

void SerialComm::send_jog(uint8_t axis, int8_t direction, int32_t speed_um_s, int32_t distance_um) {
    const protocol::JogAxisPayload jog{axis, direction, speed_um_s, distance_um};
    if (protocol::validate_jog(jog) != protocol::CommandResult::Accepted) {
        emit packet_error("Invalid jog parameters");
        return;
    }
    const auto payload = protocol::encode_jog(jog);
    send_packet(protocol::MsgId::CMD_JOG_AXIS, payload.data(), protocol::JOG_PAYLOAD_SIZE);
}

void SerialComm::send_solenoid_air(uint16_t duration_ms) {
    std::array<uint8_t, 2> payload{};
    protocol::put_u16_le(payload.data(), duration_ms);
    send_packet(protocol::MsgId::CMD_SOLENOID_AIR, payload.data(), 2);
}

void SerialComm::send_calibration(int32_t scale_nm) {
    std::array<uint8_t, 4> payload{};
    protocol::put_i32_le(payload.data(), scale_nm);
    send_packet(protocol::MsgId::CMD_SET_CALIB, payload.data(), 4);
}

void SerialComm::parse_byte(uint8_t byte) {
    const bool was_partial = parser_.partial();
    protocol::Packet packet;
    const auto result = parser_.consume(byte, packet);
    if (!was_partial && parser_.partial()) frame_started_ms_ = now_ms();
    if (result == protocol::ParseResult::Complete) dispatch_packet(packet);
    else if (result == protocol::ParseResult::BadCrc || result == protocol::ParseResult::InvalidLength) {
        invalidate_telemetry();
        emit packet_error("Invalid reference frame length or CRC");
    }
}

void SerialComm::dispatch_packet(const protocol::Packet& packet) {
    if (packet.msg_id == static_cast<uint8_t>(protocol::MsgId::TELEM_STATUS)) {
        if (!telemetry_status_.observe(packet, now_ms())) {
            invalidate_telemetry();
            emit packet_error("Invalid, non-reference or replayed telemetry");
            return;
        }
        last_fresh_ = true;
        emit telemetry_validity_changed(true);
        emit telemetry_received(telemetry_status_.latest());
    } else if (packet.msg_id == static_cast<uint8_t>(protocol::MsgId::RESP_NACK) && packet.len == 1) {
        emit packet_error(QString("Reference command rejected (result %1)").arg(packet.payload[0]));
    }
}

} // namespace starz::hmi
