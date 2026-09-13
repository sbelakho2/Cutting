// starz-ctl: serial command-line tool for the STARZ machine host link.
// The machine runs without it. It is a commissioning/service link for loading
// the persisted recipe, jogging, calibration and diagnostic telemetry.
//
//   starz-ctl <device> status
//   starz-ctl <device> watch [period_ms]
//   starz-ctl <device> load <length_mm> <lead_mm> <tail_mm> <depth_mm> <wire_mm> <qty>
//   starz-ctl <device> clear | start | stop | reset | home
//   starz-ctl <device> jog <axis 0-3> <+|-> <distance_mm> <speed_mm_s>
//   starz-ctl <device> calib <left_counts_per_m> <right_counts_per_m>
#include "protocol.hpp"
#include <chrono>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

using namespace starz::fw;

namespace {

int open_port(const char* dev) {
    const int fd = ::open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) { std::perror(dev); return -1; }
    termios tio{};
    if (tcgetattr(fd, &tio) != 0) { std::perror("tcgetattr"); ::close(fd); return -1; }
    cfmakeraw(&tio);
    cfsetispeed(&tio, B115200); cfsetospeed(&tio, B115200); // USB-CDC ignores the rate
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cc[VMIN] = 0; tio.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tio) != 0) { std::perror("tcsetattr"); ::close(fd); return -1; }
    tcflush(fd, TCIOFLUSH);
    return fd;
}

bool write_all(int fd, const std::uint8_t* p, std::size_t n) {
    while (n) {
        const ssize_t w = ::write(fd, p, n);
        if (w < 0) { if (errno == EAGAIN) { pollfd pf{fd, POLLOUT, 0}; ::poll(&pf, 1, 100); continue; } return false; }
        p += w; n -= static_cast<std::size_t>(w);
    }
    return true;
}

// Sends req, waits up to timeout_ms for a reply with the same seq.
bool transact(int fd, const proto::Frame& req, proto::Frame& reply, int timeout_ms = 500) {
    std::uint8_t buf[proto::max_frame];
    const std::size_t n = proto::encode(req, buf, sizeof buf);
    if (!n || !write_all(fd, buf, n)) return false;
    proto::Decoder d;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    for (;;) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
        if (remaining <= 0) return false;
        pollfd pf{fd, POLLIN, 0};
        const int r = ::poll(&pf, 1, static_cast<int>(remaining));
        if (r < 0) { if (errno == EINTR) continue; return false; }
        if (r == 0) return false;
        std::uint8_t b[64];
        const ssize_t got = ::read(fd, b, sizeof b);
        if (got <= 0) { if (errno == EAGAIN) continue; return false; }
        for (ssize_t i = 0; i < got; ++i)
            if (d.feed(b[i]) && d.frame().seq == req.seq) { reply = d.frame(); return true; }
    }
}

std::uint8_t next_seq() { static std::uint8_t s = 0; return ++s; }

bool command_reply(const proto::Frame& frame) {
    return (frame.msg == proto::Msg::Ack || frame.msg == proto::Msg::Nack) && frame.len == 1;
}

bool parse_mm_to_um(const char* text, std::int32_t& out) {
    if (text[0] == '\0' || std::isspace(static_cast<unsigned char>(text[0]))) return false;
    char* end = nullptr;
    errno = 0;
    const double mm = std::strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !std::isfinite(mm)) return false;
    const double um = std::round(mm * 1000.0);
    if (um < static_cast<double>(std::numeric_limits<std::int32_t>::min()) ||
        um > static_cast<double>(std::numeric_limits<std::int32_t>::max())) return false;
    out = static_cast<std::int32_t>(um);
    return true;
}

bool parse_i32(const char* text, std::int32_t& out) {
    if (text[0] == '\0' || std::isspace(static_cast<unsigned char>(text[0]))) return false;
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < std::numeric_limits<std::int32_t>::min() || value > std::numeric_limits<std::int32_t>::max()) return false;
    out = static_cast<std::int32_t>(value);
    return true;
}

bool parse_u32(const char* text, std::uint32_t& out) {
    if (text[0] == '\0' || text[0] == '-' || text[0] == '+' || std::isspace(static_cast<unsigned char>(text[0]))) return false;
    char* end = nullptr;
    errno = 0;
    const unsigned long value = std::strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > std::numeric_limits<std::uint32_t>::max()) return false;
    out = static_cast<std::uint32_t>(value);
    return true;
}

void print_status(const Telemetry& t) {
    std::printf("state=%s phase=%s faults=0x%04x", proto::state_name(t.state), proto::phase_name(t.phase), t.faults);
    for (unsigned b = 0; b < 16; ++b) if (t.faults & (1u << b)) std::printf(" [%s]", proto::fault_name(b));
    std::printf("\ninputs: start=%d stop_ok=%d reset=%d safety_ok=%d home_front=%d home_rear=%d alm=%d%d%d%d\n",
                t.inputs.start, t.inputs.stop_ok, t.inputs.reset, t.inputs.safety_ok, t.inputs.home_front, t.inputs.home_rear,
                t.inputs.alm[0], t.inputs.alm[1], t.inputs.alm[2], t.inputs.alm[3]);
    std::printf("outputs: green=%d red=%d buzzer=%d drives_disabled=%d\n", t.outputs.led_green, t.outputs.led_red, t.outputs.buzzer, t.outputs.drives_disabled);
    std::printf("job_loaded=%d pieces_done=%u scrap=%u residual_last=%d um worst=%d um\n", t.job_loaded, t.counters.pieces_done,
                t.counters.pieces_scrap, t.counters.last_residual_um, t.counters.worst_residual_um);
    if (t.job_loaded) {
        std::printf("retained_recipe: length=%.3f mm lead=%.3f mm tail=%.3f mm depth=%.3f mm wire=%.3f mm quantity=%u\n",
                    static_cast<double>(t.job.length_um) / 1000.0,
                    static_cast<double>(t.job.strip_lead_um) / 1000.0,
                    static_cast<double>(t.job.strip_tail_um) / 1000.0,
                    static_cast<double>(t.job.strip_depth_um) / 1000.0,
                    static_cast<double>(t.job.wire_diameter_um) / 1000.0,
                    t.job.quantity);
    } else {
        std::printf("retained_recipe: none\n");
    }
    std::printf("blade_um=[%d %d] feed_um=[%lld %lld] enc=[%lld %lld] counts_per_m=[%d %d] uptime=%llu ms\n",
                t.blade_um[0], t.blade_um[1], static_cast<long long>(t.feed_um[0]), static_cast<long long>(t.feed_um[1]),
                static_cast<long long>(t.encoder_counts[0]), static_cast<long long>(t.encoder_counts[1]),
                t.settings.counts_per_metre[0], t.settings.counts_per_metre[1],
                static_cast<unsigned long long>(t.uptime_us / 1000));
}

int simple(int fd, proto::Msg m) {
    proto::Frame req{}; req.msg = m; req.seq = next_seq();
    proto::Frame rep{};
    if (!transact(fd, req, rep) || !command_reply(rep)) { std::fprintf(stderr, "invalid reply\n"); return 3; }
    const auto r = static_cast<CommandResult>(rep.payload[0]);
    std::printf("%s\n", proto::result_name(r));
    return r == CommandResult::Accepted ? 0 : 1;
}

int usage() {
    std::fprintf(stderr,
        "usage: starz-ctl <device> status|watch [ms]|load L lead tail depth wire qty (mm)|clear|start|stop|reset|home|\n"
        "                 jog <axis 0-3> <+|-> <mm> <mm/s>|calib <left> <right>\n");
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) return usage();
    const int fd = open_port(argv[1]);
    if (fd < 0) return 3;
    const std::string cmd = argv[2];

    {   // Handshake.
        proto::Frame req{}; req.msg = proto::Msg::Hello; req.seq = next_seq();
        proto::Frame rep{};
        if (!transact(fd, req, rep) || rep.msg != proto::Msg::HelloAck || rep.len < 2 || rep.len > proto::max_payload) { std::fprintf(stderr, "no valid HELLO_ACK from %s\n", argv[1]); return 3; }
        if (rep.payload[0] != proto::protocol_version) { std::fprintf(stderr, "protocol version %u, expected %u\n", rep.payload[0], proto::protocol_version); return 3; }
        const auto* end = static_cast<const std::uint8_t*>(std::memchr(rep.payload + 1, 0, rep.len - 1));
        if (end == nullptr) { std::fprintf(stderr, "unterminated machine revision\n"); return 3; }
        if (cmd != "watch") std::printf("machine %.*s\n", static_cast<int>(end - (rep.payload + 1)), reinterpret_cast<const char*>(rep.payload + 1));
    }

    if (cmd == "status" || cmd == "watch") {
        if ((cmd == "status" && argc != 3) || (cmd == "watch" && argc != 3 && argc != 4)) return usage();
        std::uint32_t period = 500;
        if (cmd == "watch" && argc > 3 && (!parse_u32(argv[3], period) || period < 50 || period > 60000)) return usage();
        for (;;) {
            proto::Frame req{}; req.msg = proto::Msg::GetStatus; req.seq = next_seq();
            proto::Frame rep{}; Telemetry t{};
            if (!transact(fd, req, rep) || rep.msg != proto::Msg::Status || !proto::decode_status(rep.payload, rep.len, t)) { std::fprintf(stderr, "no status\n"); return 3; }
            print_status(t);
            if (cmd == "status") return 0;
            std::printf("--\n"); std::fflush(stdout);
            ::usleep(static_cast<useconds_t>(period) * 1000u);
        }
    }
    if (cmd == "load") {
        if (argc != 9) return usage();
        Job j{};
        if (!parse_mm_to_um(argv[3], j.length_um) || !parse_mm_to_um(argv[4], j.strip_lead_um) ||
            !parse_mm_to_um(argv[5], j.strip_tail_um) || !parse_mm_to_um(argv[6], j.strip_depth_um) ||
            !parse_mm_to_um(argv[7], j.wire_diameter_um) || !parse_u32(argv[8], j.quantity)) return usage();
        const JobError e = validate(j);
        if (e != JobError::None) { std::fprintf(stderr, "job rejected locally (JobError %d)\n", static_cast<int>(e)); return 1; }
        proto::Frame req{}; req.msg = proto::Msg::LoadJob; req.seq = next_seq(); req.len = proto::load_job_len;
        proto::encode_job(j, req.payload);
        proto::Frame rep{};
        if (!transact(fd, req, rep) || !command_reply(rep)) { std::fprintf(stderr, "invalid reply\n"); return 3; }
        std::printf("%s\n", proto::result_name(static_cast<CommandResult>(rep.payload[0])));
        return rep.msg == proto::Msg::Ack ? 0 : 1;
    }
    if (cmd == "clear") return simple(fd, proto::Msg::ClearJob);
    if (cmd == "start") return simple(fd, proto::Msg::Start);
    if (cmd == "stop") return simple(fd, proto::Msg::Stop);
    if (cmd == "reset") return simple(fd, proto::Msg::Reset);
    if (cmd == "home") return simple(fd, proto::Msg::Home);
    if (cmd == "jog") {
        if (argc != 7) return usage();
        std::uint32_t axis = 0;
        std::int32_t distance = 0, speed = 0;
        if (!parse_u32(argv[3], axis) || axis >= starz::ctrl::axis::count ||
            (std::strcmp(argv[4], "+") != 0 && std::strcmp(argv[4], "-") != 0) ||
            !parse_mm_to_um(argv[5], distance) || !parse_mm_to_um(argv[6], speed) || distance <= 0 || speed <= 0) return usage();
        proto::Frame req{}; req.msg = proto::Msg::Jog; req.seq = next_seq(); req.len = proto::jog_len;
        req.payload[0] = static_cast<std::uint8_t>(axis);
        req.payload[1] = argv[4][0] == '+' ? 1 : 0;
        proto::put_i32(req.payload + 2, distance);
        proto::put_i32(req.payload + 6, speed);
        proto::Frame rep{};
        if (!transact(fd, req, rep) || !command_reply(rep)) { std::fprintf(stderr, "invalid reply\n"); return 3; }
        std::printf("%s\n", proto::result_name(static_cast<CommandResult>(rep.payload[0])));
        return rep.msg == proto::Msg::Ack ? 0 : 1;
    }
    if (cmd == "calib") {
        if (argc != 5) return usage();
        std::int32_t left = 0, right = 0;
        if (!parse_i32(argv[3], left) || !parse_i32(argv[4], right)) return usage();
        proto::Frame req{}; req.msg = proto::Msg::SetCalibration; req.seq = next_seq(); req.len = proto::calibration_len;
        proto::put_i32(req.payload, left); proto::put_i32(req.payload + 4, right);
        proto::Frame rep{};
        if (!transact(fd, req, rep) || !command_reply(rep)) { std::fprintf(stderr, "invalid reply\n"); return 3; }
        std::printf("%s\n", proto::result_name(static_cast<CommandResult>(rep.payload[0])));
        return rep.msg == proto::Msg::Ack ? 0 : 1;
    }
    return usage();
}
