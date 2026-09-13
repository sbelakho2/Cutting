#include "reference_fixture.hpp"
#include "../../hmi/telemetry_status.hpp"
#include <bit>
#include <iostream>
#include <limits>

namespace {
using namespace starz;
using namespace starz::tests;

void test_crc_endian_and_serialization_bounds() {
    const std::array<uint8_t, 9> known{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CHECK(protocol::compute_crc16(known.data(), known.size()) == 0x29B1);
    for (unsigned length = 0; length <= 255; ++length) {
        auto value = packet(MsgId::CMD_PING, 7);
        value.len = static_cast<uint8_t>(length);
        value.payload.fill(0x42);
        std::array<uint8_t, protocol::MAX_FRAME_SIZE + 2> bytes{};
        bytes.fill(0xCC);
        const auto written = value.serialize(bytes.data() + 1, protocol::MAX_FRAME_SIZE);
        CHECK(bytes.front() == 0xCC && bytes.back() == 0xCC);
        if (length > protocol::MAX_PAYLOAD_SIZE) {
            CHECK(written == 0);
            CHECK(std::all_of(bytes.begin(), bytes.end(), [](uint8_t byte) { return byte == 0xCC; }));
        } else {
            CHECK(written == 7U + length);
            CHECK(bytes[1] == 0xA5 && bytes[2] == 0x5A);
            CHECK(value.serialize(bytes.data(), written - 1) == 0);
        }
        CHECK(value.serialize(nullptr, 1000) == 0);
    }

    const protocol::JobStartPayload source{0x01020304, -2, std::numeric_limits<int32_t>::min(), 0x10203040, 0x89ABCDEFU};
    const auto encoded = protocol::encode_job(source);
    CHECK(encoded[0] == 4 && encoded[1] == 3 && encoded[2] == 2 && encoded[3] == 1);
    CHECK(encoded[4] == 0xFE && encoded[7] == 0xFF);
    CHECK(encoded[8] == 0 && encoded[11] == 0x80);
    CHECK(encoded[16] == 0xEF && encoded[19] == 0x89);
    protocol::JobStartPayload decoded{};
    CHECK(protocol::decode_job(encoded.data(), encoded.size(), decoded));
    CHECK(decoded.wire_length_um == source.wire_length_um);
    CHECK(decoded.lead_strip_um == source.lead_strip_um);
    CHECK(decoded.tail_strip_um == source.tail_strip_um);
    CHECK(decoded.blade_strip_depth_um == source.blade_strip_depth_um);
    CHECK(decoded.target_quantity == source.target_quantity);
    CHECK(!protocol::decode_job(nullptr, protocol::JOB_PAYLOAD_SIZE, decoded));

    const protocol::JogAxisPayload jog{3, -1, 12345, 6789};
    const auto jog_bytes = protocol::encode_jog(jog);
    CHECK(jog_bytes[1] == 0xFF);
    protocol::JogAxisPayload decoded_jog{};
    CHECK(protocol::decode_jog(jog_bytes.data(), jog_bytes.size(), decoded_jog));
    CHECK(decoded_jog.axis_id == 3 && decoded_jog.direction == -1);
    CHECK(decoded_jog.speed_um_s == 12345 && decoded_jog.distance_um == 6789);

    const protocol::TelemetryPayload telemetry{6, 0xE0, -100, 200, 300, 400, 0x01020304U, 9, 0x1234, 0x5678};
    const auto telemetry_bytes = protocol::encode_telemetry(telemetry);
    CHECK(telemetry_bytes.size() == 30);
    CHECK(telemetry_bytes[18] == 4 && telemetry_bytes[21] == 1);
    CHECK(telemetry_bytes[26] == 0x34 && telemetry_bytes[27] == 0x12);
    CHECK(telemetry_bytes[28] == 0x78 && telemetry_bytes[29] == 0x56);
    protocol::TelemetryPayload decoded_telemetry{};
    CHECK(protocol::decode_telemetry(telemetry_bytes.data(), telemetry_bytes.size(), decoded_telemetry));
    CHECK(protocol::encode_telemetry(decoded_telemetry) == telemetry_bytes);
    for (size_t length = 0; length < 100; ++length) {
        CHECK(protocol::decode_job(encoded.data(), length, decoded) == (length == 20));
        CHECK(protocol::decode_jog(jog_bytes.data(), length, decoded_jog) == (length == 10));
        CHECK(protocol::decode_telemetry(telemetry_bytes.data(), length, decoded_telemetry) == (length == 30));
    }
    const auto framed = packet(MsgId::TELEM_STATUS, 2, telemetry_bytes);
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> output{};
    CHECK(framed.serialize(output.data(), output.size()) == 37);
}

void test_parser_and_deterministic_noise() {
    protocol::PacketParser parser;
    protocol::Packet received;
    auto valid = packet(MsgId::CMD_PING, 3);
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> bytes{};
    const auto size = valid.serialize(bytes.data(), bytes.size());
    CHECK(parser.consume(0xA5, received) == protocol::ParseResult::Incomplete);
    for (size_t index = 0; index < size; ++index) {
        CHECK(parser.consume(bytes[index], received) == (index + 1 == size ? protocol::ParseResult::Complete : protocol::ParseResult::Incomplete));
    }
    CHECK(received.seq == 3 && received.len == 0);
    CHECK(!parser.partial());
    bytes[size - 1] ^= 0x01;
    for (size_t index = 0; index < size; ++index) {
        CHECK(parser.consume(bytes[index], received) == (index + 1 == size ? protocol::ParseResult::BadCrc : protocol::ParseResult::Incomplete));
    }
    for (uint8_t byte : {uint8_t{0xA5}, uint8_t{0x5A}, uint8_t{0x10}, uint8_t{0}}) parser.consume(byte, received);
    CHECK(parser.consume(65, received) == protocol::ParseResult::InvalidLength);
    CHECK(!parser.partial());

    firmware::FirmwareController controller;
    uint32_t random = 0x61359A7BU;
    for (int index = 0; index < 200000; ++index) {
        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        const auto byte = static_cast<uint8_t>(random);
        parser.consume(byte, received);
        controller.process_incoming_byte(byte);
        if (index % 1000 == 0) controller.update_tick(1000);
    }
    CHECK(!controller.reference_motion_active());
    CHECK(!controller.hardware_outputs_enabled());
    CHECK(controller.get_telemetry().completed_pcs == 0);
    controller.update_tick(static_cast<uint32_t>(protocol::FRAME_TIMEOUT_US));
    CHECK(deliver(controller, packet(MsgId::CMD_PING, 64)) == CommandResult::Accepted);
}

void test_exact_command_lengths_and_binary_validation() {
    const protocol::JobStartPayload job{500000, 10000, 10000, 3000, 2};
    const protocol::JogAxisPayload jog{0, 1, 10000, 1000};
    for (unsigned length = 0; length <= protocol::MAX_PAYLOAD_SIZE; ++length) {
        ReferenceFixture fixture;
        auto job_packet = packet(MsgId::CMD_JOB_START, fixture.next_sequence++, protocol::encode_job(job));
        job_packet.len = static_cast<uint8_t>(length);
        CHECK(deliver(fixture.controller, job_packet) == (length == 20 ? CommandResult::NotCommissioned : CommandResult::InvalidLength));
        CHECK(fixture.controller.get_state() == MachineState::IDLE);
        auto jog_packet = packet(MsgId::CMD_JOG_AXIS, fixture.next_sequence++, protocol::encode_jog(jog));
        jog_packet.len = static_cast<uint8_t>(length);
        CHECK(deliver(fixture.controller, jog_packet) == (length == 10 ? CommandResult::Accepted : CommandResult::InvalidLength));
        CHECK(fixture.controller.get_telemetry().completed_pcs == 0);
    }
    for (const auto id : {MsgId::CMD_PING, MsgId::CMD_JOB_PAUSE, MsgId::CMD_JOB_RESUME, MsgId::CMD_JOB_ABORT,
                          MsgId::CMD_RECOVER, MsgId::CMD_HOME_ALL, MsgId::CMD_SOLENOID_DOOR}) {
        ReferenceFixture fixture;
        auto malformed = packet(id, fixture.next_sequence++);
        malformed.len = 1;
        CHECK(deliver(fixture.controller, malformed) == CommandResult::InvalidLength);
        CHECK(fixture.controller.get_state() == MachineState::IDLE);
    }
    ReferenceFixture fixture;
    auto invalid_job = job;
    invalid_job.lead_strip_um = std::numeric_limits<int32_t>::max();
    invalid_job.tail_strip_um = std::numeric_limits<int32_t>::max();
    CHECK(deliver(fixture.controller, packet(MsgId::CMD_JOB_START, fixture.next_sequence++, protocol::encode_job(invalid_job))) == CommandResult::InvalidArgument);
    auto invalid_jog = jog;
    invalid_jog.direction = std::numeric_limits<int8_t>::min();
    invalid_jog.distance_um = std::numeric_limits<int32_t>::min();
    CHECK(deliver(fixture.controller, packet(MsgId::CMD_JOG_AXIS, fixture.next_sequence++, protocol::encode_jog(invalid_jog))) == CommandResult::InvalidArgument);
    for (const auto id : {MsgId::CMD_SET_CALIB, MsgId::CMD_SOLENOID_AIR}) {
        auto payload = packet(id, fixture.next_sequence++);
        CHECK(deliver(fixture.controller, payload) == CommandResult::InvalidLength);
        payload.len = id == MsgId::CMD_SET_CALIB ? 4 : 2;
        CHECK(deliver(fixture.controller, payload) == CommandResult::InvalidArgument);
        payload.payload[0] = 1;
        CHECK(deliver(fixture.controller, payload) == CommandResult::NotCommissioned);
        payload.len = static_cast<uint8_t>(payload.len + 1);
        CHECK(deliver(fixture.controller, payload) == CommandResult::InvalidLength);
    }
    CHECK(deliver(fixture.controller, packet(static_cast<MsgId>(0x77), fixture.next_sequence++)) == CommandResult::Unsupported);
    CHECK(fixture.controller.get_state() == MachineState::IDLE);

    protocol::Packet response;
    CHECK(deliver(fixture.controller, packet(MsgId::CMD_JOB_START, fixture.next_sequence, protocol::encode_job(job))) == CommandResult::NotCommissioned);
    CHECK(fixture.controller.take_response(response));
    CHECK(response.msg_id == static_cast<uint8_t>(MsgId::RESP_NACK));
    CHECK(response.seq == fixture.next_sequence && response.len == 1);
    CHECK(response.payload[0] == static_cast<uint8_t>(CommandResult::NotCommissioned));
    CHECK(!fixture.controller.take_response(response));
}

void test_heartbeat_replay_and_non_ping_traffic() {
    protocol::SequenceWindow sequences;
    CHECK(sequences.accept(254));
    CHECK(sequences.accept(255));
    CHECK(sequences.accept(0));
    CHECK(!sequences.accept(0));
    CHECK(!sequences.accept(255));
    CHECK(sequences.accept(127));
    CHECK(!sequences.accept(255)); // Ambiguous half-window jump is rejected.

    ReferenceFixture duplicate;
    for (int index = 0; index < 749; ++index) {
        duplicate.advance(1000, false, false);
        CHECK(deliver(duplicate.controller, packet(MsgId::CMD_PING, 0)) == CommandResult::Replay);
    }
    duplicate.advance(999, false, false);
    CHECK(!has_fault(duplicate.controller, Fault::HeartbeatLost));
    duplicate.advance(1, false, false);
    CHECK(has_fault(duplicate.controller, Fault::HeartbeatLost));
    duplicate.ping();
    CHECK(has_fault(duplicate.controller, Fault::HeartbeatLost));

    ReferenceFixture traffic;
    for (int index = 0; index < 750; ++index) {
        if (index % 10 == 0) {
            const auto payload = protocol::encode_job({500000, 10000, 10000, 3000, 1});
            CHECK(deliver(traffic.controller, packet(MsgId::CMD_JOB_START, traffic.next_sequence++, payload)) == CommandResult::NotCommissioned);
            auto malformed_ping = packet(MsgId::CMD_PING, traffic.next_sequence++);
            malformed_ping.len = 1;
            CHECK(deliver(traffic.controller, malformed_ping) == CommandResult::InvalidLength);
        }
        traffic.advance(1000, false, false);
    }
    CHECK(has_fault(traffic.controller, Fault::HeartbeatLost));

    ReferenceFixture bad_crc;
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> bytes{};
    const auto ping = packet(MsgId::CMD_PING, 1);
    const auto length = ping.serialize(bytes.data(), bytes.size());
    bytes[length - 1] ^= 1;
    for (int index = 0; index < 750; ++index) {
        for (size_t offset = 0; offset < length; ++offset) bad_crc.controller.process_incoming_byte(bytes[offset]);
        CHECK(bad_crc.controller.last_result() == CommandResult::MalformedFrame);
        bad_crc.advance(1000, false, false);
    }
    CHECK(has_fault(bad_crc.controller, Fault::HeartbeatLost));

    ReferenceFixture replay;
    auto jog = packet(MsgId::CMD_JOG_AXIS, replay.next_sequence++, protocol::encode_jog({0, 1, 10000, 1000}));
    CHECK(deliver(replay.controller, jog) == CommandResult::Accepted);
    CHECK(deliver(replay.controller, jog) == CommandResult::Replay);
    replay.inputs.safety_relay_ok = false;
    replay.sample();
    replay.inputs.safety_relay_ok = true;
    replay.sample();
    CHECK(replay.controller.trigger_recovery() == CommandResult::Accepted);
    CHECK(deliver(replay.controller, jog) == CommandResult::Replay);
    CHECK(!replay.controller.reference_motion_active());
    jog.seq = replay.next_sequence++;
    CHECK(deliver(replay.controller, jog) == CommandResult::Accepted);
    CHECK(deliver(replay.controller, packet(MsgId::CMD_JOB_ABORT, 0)) == CommandResult::Accepted);
    CHECK(!replay.controller.reference_motion_active());
    CHECK(deliver(replay.controller, packet(MsgId::CMD_JOB_ABORT, 0)) == CommandResult::Accepted);
    replay.controller.init();
    CHECK(deliver(replay.controller, jog) == CommandResult::Replay);
}

void test_partial_frame_expiry_and_resynchronization() {
    ReferenceFixture fixture;
    const auto job = packet(MsgId::CMD_JOB_START, 1, protocol::encode_job({500000, 10000, 10000, 3000, 2}));
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> bytes{};
    CHECK(job.serialize(bytes.data(), bytes.size()) == 27);
    for (size_t index = 0; index < 8; ++index) fixture.controller.process_incoming_byte(bytes[index]);
    for (int index = 0; index < 100; ++index) fixture.advance(1000, false, false);
    CHECK(fixture.controller.last_result() == CommandResult::MalformedFrame);
    CHECK(fixture.controller.get_state() == MachineState::IDLE);
    CHECK(deliver(fixture.controller, packet(MsgId::CMD_PING, 2)) == CommandResult::Accepted);
    CHECK(fixture.controller.trigger_job_start(500000, 10000, 10000, 3000, 2) == CommandResult::NotCommissioned);
}

void test_hmi_freshness_unknown_replay_and_clamping() {
    hmi::TelemetryStatus status;
    ReferenceFixture fixture;
    auto telemetry = fixture.controller.get_telemetry();
    auto value = packet(MsgId::TELEM_STATUS, 0, protocol::encode_telemetry(telemetry));
    CHECK(!status.fresh(0));
    CHECK(!status.observe(value, 0));
    status.set_connected(true);
    CHECK(!status.fresh(0));
    CHECK(status.observe(value, 100));
    CHECK(status.fresh(100));
    CHECK(status.fresh(849));
    CHECK(!status.fresh(850));
    CHECK(!status.fresh(99)); // Regressed clocks are not considered fresh.
    CHECK(!status.observe(value, 900));
    CHECK(!status.fresh(900));
    value.seq = 1;
    CHECK(status.observe(value, 901));
    value.seq = 2;
    value.len = 31;
    CHECK(!status.observe(value, 902));
    CHECK(!status.fresh(902));
    value.len = 30;
    CHECK(status.observe(value, 903));
    status.set_connected(false);
    CHECK(!status.fresh(903));
    status.set_connected(true);
    CHECK(!status.fresh(903));
    CHECK(status.observe(value, 904));
    status.invalidate();
    CHECK(!status.fresh(904));

    for (const auto bad_state : {7U, 128U, 255U}) {
        telemetry.state = static_cast<uint8_t>(bad_state);
        CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    }
    telemetry = fixture.controller.get_telemetry();
    telemetry.safety_flags &= static_cast<uint8_t>(~protocol::REFERENCE_ONLY_FLAG);
    CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    telemetry = fixture.controller.get_telemetry();
    telemetry.completed_pcs = 1;
    CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    telemetry.completed_pcs = 0;
    telemetry.target_pcs = 1;
    CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    telemetry.target_pcs = 0;
    telemetry.cycle_time_ms = 1;
    CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    telemetry.cycle_time_ms = 0;
    telemetry.encoder_meas_um = std::numeric_limits<int32_t>::max();
    CHECK(!hmi::TelemetryStatus::valid_reference_telemetry(telemetry));
    telemetry.encoder_meas_um = protocol::UNKNOWN_POSITION_UM;
    CHECK(hmi::TelemetryStatus::valid_reference_telemetry(telemetry));

    CHECK(hmi::TelemetryStatus::progress_percent(123, 0) == 0);
    CHECK(hmi::TelemetryStatus::progress_percent(1, 3) == 33);
    CHECK(hmi::TelemetryStatus::progress_percent(50, 100) == 50);
    CHECK(hmi::TelemetryStatus::progress_percent(std::numeric_limits<uint32_t>::max(), 1) == 100);
    CHECK(hmi::TelemetryStatus::progress_percent(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()) == 100);
}
} // namespace

int main() {
    try {
        test_crc_endian_and_serialization_bounds();
        test_parser_and_deterministic_noise();
        test_exact_command_lengths_and_binary_validation();
        test_heartbeat_replay_and_non_ping_traffic();
        test_partial_frame_expiry_and_resynchronization();
        test_hmi_freshness_unknown_replay_and_clamping();
        std::cout << "PASS: 6 reference protocol/HMI-policy test groups; no hardware or security certification.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}