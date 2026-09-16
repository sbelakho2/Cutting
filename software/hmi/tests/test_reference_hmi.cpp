#include "../mainwindow.hpp"
#include "../../firmware/tests/test_support.hpp"
#include <QApplication>
#include <QPixmap>
#include <iostream>

namespace {
void check_disabled_controls(starz::hmi::MainWindow& window) {
    for (const auto* name : {"BtnStart", "BtnPause", "BtnHome", "BtnDoor", "BtnAir", "BtnAbort"}) {
        const auto* button = window.findChild<QPushButton*>(name);
        CHECK(button != nullptr);
        CHECK(!button->isEnabled());
    }
}
} // namespace

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    try {
        starz::hmi::MainWindow window;
        CHECK(window.windowTitle().contains("NOT FOR MACHINE OPERATION"));
        const auto* warning = window.findChild<QLabel*>("PrototypeWarning");
        CHECK(warning != nullptr);
        CHECK(warning->text().contains("REFERENCE PROTOTYPE"));
        CHECK(warning->text().contains("NOT FOR MACHINE OPERATION"));
        const auto* badge = window.findChild<QLabel*>("SafetyBadge");
        CHECK(badge != nullptr);
        CHECK(badge->text().contains("UNKNOWN"));
        check_disabled_controls(window);

        auto* comm = window.findChild<starz::hmi::SerialComm*>();
        CHECK(comm != nullptr);
        CHECK(!comm->has_fresh_telemetry());
        int denied = 0;
        QObject::connect(comm, &starz::hmi::SerialComm::packet_error, &window, [&denied](const QString&) { ++denied; });
        // Public methods cannot bypass the disabled widgets or missing telemetry.
        comm->send_job_start(500000, 10000, 10000, 3000, 2);
        comm->send_jog(0, 1, 10000, 1000);
        comm->send_home();
        comm->send_solenoid_door();
        comm->send_solenoid_air(80);
        comm->send_calibration(48828);
        comm->send_job_resume();
        CHECK(denied == 7);
        CHECK(!comm->is_connected());

        // A status signal alone is not fresh validated transport data.
        comm->telemetry_validity_changed(true);
        comm->telemetry_received({3, 0xCF, 100, 100, 0, 0, 0, 0, 0, 0});
        CHECK(badge->text().contains("UNKNOWN"));
        check_disabled_controls(window);
        comm->disconnect_port();
        CHECK(!comm->has_fresh_telemetry());
        CHECK(badge->text().contains("UNKNOWN"));
        check_disabled_controls(window);

        // Exercise Qt painting in an offscreen host process, not a browser/device.
        QPixmap image(window.size());
        window.render(&image);
        CHECK(!image.isNull());
        std::cout << "PASS: prototype warning, disabled controls, stale/unknown display, serial command gate, offscreen paint.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}