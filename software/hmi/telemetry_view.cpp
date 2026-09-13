#include "telemetry_view.hpp"
#include <QPainterPath>

namespace starz::hmi {

TelemetryView::TelemetryView(QWidget* parent) : QWidget(parent) {
    setMinimumSize(480, 160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void TelemetryView::update_telemetry(const protocol::TelemetryPayload& telem) {
    current_telem_ = telem;
    fresh_ = true;
    update();
}

void TelemetryView::invalidate_telemetry() {
    current_telem_ = {};
    fresh_ = false;
    update();
}

void TelemetryView::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), QColor("#161B22"));
    p.setPen(QPen(QColor("#30363D"), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    int w = width();
    int h = height();
    int mid_y = h / 2;

    // Draw Wire Track centerline
    p.setPen(QPen(QColor("#484F58"), 2, Qt::DashLine));
    p.drawLine(20, mid_y, w - 20, mid_y);

    // Draw Left Feed Roller Cassette
    int l_roller_x = w * 25 / 100;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#E5E7EB"));
    p.drawRoundedRect(l_roller_x - 12, mid_y - 28, 24, 18, 3, 3);
    p.drawRoundedRect(l_roller_x - 12, mid_y + 10, 24, 18, 3, 3);

    // Draw Right Feed Roller Cassette
    int r_roller_x = w * 75 / 100;
    p.drawRoundedRect(r_roller_x - 12, mid_y - 28, 24, 18, 3, 3);
    p.drawRoundedRect(r_roller_x - 12, mid_y + 10, 24, 18, 3, 3);

    // Draw Cutting Center Blades (Star Red: #E10600)
    int cut_x = w / 2;
    p.setBrush(QColor("#E10600"));
    // Upper V-Blade
    QPainterPath upper_v;
    upper_v.moveTo(cut_x - 14, mid_y - 35);
    upper_v.lineTo(cut_x + 14, mid_y - 35);
    upper_v.lineTo(cut_x, mid_y - 8);
    upper_v.closeSubpath();
    p.drawPath(upper_v);

    // Lower V-Blade
    QPainterPath lower_v;
    lower_v.moveTo(cut_x - 14, mid_y + 35);
    lower_v.lineTo(cut_x + 14, mid_y + 35);
    lower_v.lineTo(cut_x, mid_y + 8);
    lower_v.closeSubpath();
    p.drawPath(lower_v);

    // Draw Active Wire Flow
    p.setPen(QPen(QColor("#38BDF8"), 4));
    p.drawLine(30, mid_y, w - 30, mid_y);

    // Telemetry Text Overlay
    p.setPen(QColor("#E5E7EB"));
    p.setFont(QFont("Monospace", 9));
    if (!fresh_) {
        p.drawText(15, h - 12, "REFERENCE ONLY — TELEMETRY UNKNOWN / STALE");
        return;
    }
    const auto position = [](int32_t um) {
        return um == protocol::UNKNOWN_POSITION_UM ? QString("UNKNOWN") : QString::number(um / 1000.0, 'f', 2);
    };
    QString delta = "UNKNOWN";
    if (current_telem_.feed_command_um != protocol::UNKNOWN_POSITION_UM && current_telem_.encoder_meas_um != protocol::UNKNOWN_POSITION_UM) {
        delta = QString::number(static_cast<qint64>(current_telem_.feed_command_um) - current_telem_.encoder_meas_um);
    }
    const QString info = QString("Ref feed: %1 mm | Injected: %2 mm | Delta: %3 um | Front: %4 mm | Rear: %5 mm")
        .arg(position(current_telem_.feed_command_um), position(current_telem_.encoder_meas_um), delta,
             position(current_telem_.blade_front_um), position(current_telem_.blade_rear_um));
    p.drawText(15, h - 12, info);
}

} // namespace starz::hmi
