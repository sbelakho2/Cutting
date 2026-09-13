#pragma once
#include <QMainWindow>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QTabWidget>
#include "serial_comm.hpp"
#include "telemetry_view.hpp"

namespace starz::hmi {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_connect_clicked();
    void on_refresh_ports_clicked();
    void on_start_job_clicked();
    void on_pause_job_clicked();
    void on_abort_job_clicked();
    void on_home_clicked();
    void on_unlock_door_clicked();
    void on_air_blast_clicked();
    void on_telemetry_updated(const protocol::TelemetryPayload& telem);
    void on_connection_status(bool connected, const QString& message);
    void on_telemetry_validity_changed(bool fresh);

private:
    void setup_ui();
    void apply_styling();
    void refresh_serial_ports();
    void update_control_availability();

    SerialComm* comm_{nullptr};
    TelemetryView* telem_view_{nullptr};

    // UI Widgets
    QComboBox* port_combo_{nullptr};
    QPushButton* btn_connect_{nullptr};
    QLabel* lbl_status_{nullptr};
    QLabel* lbl_safety_state_{nullptr};

    // Job Parameters
    QDoubleSpinBox* spin_length_{nullptr};
    QDoubleSpinBox* spin_lead_strip_{nullptr};
    QDoubleSpinBox* spin_tail_strip_{nullptr};
    QDoubleSpinBox* spin_blade_depth_{nullptr};
    QSpinBox* spin_quantity_{nullptr};

    // Controls
    QPushButton* btn_start_{nullptr};
    QPushButton* btn_pause_{nullptr};
    QPushButton* btn_abort_{nullptr};
    QPushButton* btn_home_{nullptr};
    QPushButton* btn_door_{nullptr};
    QPushButton* btn_air_{nullptr};

    // Telemetry & Progress
    QLabel* lbl_pieces_count_{nullptr};
    QLabel* lbl_cycle_time_{nullptr};
    QLabel* lbl_throughput_{nullptr};
    QProgressBar* progress_bar_{nullptr};
};

} // namespace starz::hmi
