#pragma once
#include <QObject>
#include <QElapsedTimer>
#include <QSerialPort>
#include <QTimer>
#include "telemetry_status.hpp"

namespace starz::hmi {

class SerialComm : public QObject {
    Q_OBJECT
public:
    explicit SerialComm(QObject* parent = nullptr);
    ~SerialComm() override;

    bool connect_to_port(const QString& port_name, qint32 baud_rate = 115200);
    void disconnect_port();
    bool is_connected() const;
    bool has_fresh_telemetry() const;

    void send_ping();
    void send_job_start(int32_t length_um, int32_t lead_strip_um, int32_t tail_strip_um, int32_t cut_depth_um, uint32_t qty);
    void send_job_pause();
    void send_job_resume();
    void send_job_abort();
    void send_home();
    void send_jog(uint8_t axis_id, int8_t dir, int32_t speed_um_s, int32_t dist_um);
    void send_solenoid_air(uint16_t duration_ms);
    void send_solenoid_door();
    void send_calibration(int32_t scale_nm);

signals:
    void connection_status_changed(bool connected, const QString& message);
    void telemetry_received(const protocol::TelemetryPayload& telem);
    void telemetry_validity_changed(bool fresh);
    void packet_error(const QString& error_msg);

private slots:
    void on_ready_read();
    void on_heartbeat_timeout();
    void on_freshness_timeout();

private:
    void send_packet(protocol::MsgId msg_id, const uint8_t* payload, uint8_t len);
    void parse_byte(uint8_t byte);
    void dispatch_packet(const protocol::Packet& pkt);
    void invalidate_telemetry();
    uint64_t now_ms() const;

    QSerialPort* serial_{nullptr};
    QTimer* heartbeat_timer_{nullptr};
    QTimer* freshness_timer_{nullptr};
    uint8_t tx_seq_{0};
    protocol::PacketParser parser_;
    TelemetryStatus telemetry_status_;
    QElapsedTimer clock_;
    uint64_t frame_started_ms_{0};
    bool last_fresh_{false};
};

} // namespace starz::hmi
