#pragma once
#include <QWidget>
#include <QPainter>
#include "../protocol/starz_protocol.hpp"

namespace starz::hmi {

class TelemetryView : public QWidget {
    Q_OBJECT
public:
    explicit TelemetryView(QWidget* parent = nullptr);

    void update_telemetry(const protocol::TelemetryPayload& telem);
    void invalidate_telemetry();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    protocol::TelemetryPayload current_telem_{};
    bool fresh_{false};
};

} // namespace starz::hmi
