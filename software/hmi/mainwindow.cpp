#include "mainwindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QStatusBar>
#include "../../shared/machine_spec.hpp"

namespace starz::hmi {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), comm_(new SerialComm(this)), telem_view_(new TelemetryView(this)) {
    setup_ui();
    apply_styling();
    refresh_serial_ports();

    connect(comm_, &SerialComm::telemetry_received, this, &MainWindow::on_telemetry_updated);
    connect(comm_, &SerialComm::connection_status_changed, this, &MainWindow::on_connection_status);
    connect(comm_, &SerialComm::telemetry_validity_changed, this, &MainWindow::on_telemetry_validity_changed);
    connect(comm_, &SerialComm::packet_error, this, [this](const QString& message) {
        statusBar()->showMessage(message, 10000);
    });

    setWindowTitle("STARZ S1 Reference Prototype — NOT FOR MACHINE OPERATION");
    on_telemetry_validity_changed(false);
    resize(1024, 720);
}

MainWindow::~MainWindow() = default;

void MainWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* main_layout = new QVBoxLayout(central);
    main_layout->setSpacing(12);
    main_layout->setContentsMargins(16, 16, 16, 16);

    // Top Brand & Header Bar
    auto* header_layout = new QHBoxLayout();
    auto* lbl_title = new QLabel("STARZ S1 REFERENCE MONITOR", this);
    lbl_title->setObjectName("TitleLabel");
    header_layout->addWidget(lbl_title);
    header_layout->addStretch();

    // Connection Bar
    port_combo_ = new QComboBox(this);
    auto* btn_refresh = new QPushButton("Refresh Ports", this);
    connect(btn_refresh, &QPushButton::clicked, this, &MainWindow::on_refresh_ports_clicked);
    btn_connect_ = new QPushButton("Connect", this);
    connect(btn_connect_, &QPushButton::clicked, this, &MainWindow::on_connect_clicked);

    lbl_status_ = new QLabel("DISCONNECTED", this);
    lbl_status_->setObjectName("StatusBadge");
    lbl_safety_state_ = new QLabel("SAFETY: UNKNOWN", this);
    lbl_safety_state_->setObjectName("SafetyBadge");

    header_layout->addWidget(port_combo_);
    header_layout->addWidget(btn_refresh);
    header_layout->addWidget(btn_connect_);
    header_layout->addWidget(lbl_status_);
    header_layout->addWidget(lbl_safety_state_);
    main_layout->addLayout(header_layout);
    auto* warning = new QLabel("REFERENCE PROTOTYPE — NOT FOR MACHINE OPERATION\n"
                              "No hardware safety, PIO, commissioned process or quality acceptance. Motion and guard controls are disabled.", this);
    warning->setObjectName("PrototypeWarning");
    warning->setWordWrap(true);
    warning->setStyleSheet("background-color: #7F1D1D; color: #FFFFFF; padding: 12px; font-weight: bold;");
    main_layout->addWidget(warning);

    // Mid Section: Job Setup & Real-Time Production Monitor
    auto* mid_layout = new QHBoxLayout();

    // Left Box: Job Parameters Form
    auto* grp_job = new QGroupBox("Job Parameters — Not Commissioned", this);
    auto* grid_job = new QGridLayout(grp_job);

    grid_job->addWidget(new QLabel("Wire Cut Length (mm):", this), 0, 0);
    spin_length_ = new QDoubleSpinBox(this);
    spin_length_->setRange(200.0, 3000.0);
    spin_length_->setValue(1000.0);
    spin_length_->setDecimals(1);
    spin_length_->setSuffix(" mm");
    grid_job->addWidget(spin_length_, 0, 1);

    grid_job->addWidget(new QLabel("Lead Strip Length (mm):", this), 1, 0);
    spin_lead_strip_ = new QDoubleSpinBox(this);
    spin_lead_strip_->setRange(3.0, 25.0);
    spin_lead_strip_->setValue(10.0);
    spin_lead_strip_->setDecimals(1);
    spin_lead_strip_->setSuffix(" mm");
    grid_job->addWidget(spin_lead_strip_, 1, 1);

    grid_job->addWidget(new QLabel("Tail Strip Length (mm):", this), 2, 0);
    spin_tail_strip_ = new QDoubleSpinBox(this);
    spin_tail_strip_->setRange(3.0, 25.0);
    spin_tail_strip_->setValue(10.0);
    spin_tail_strip_->setDecimals(1);
    spin_tail_strip_->setSuffix(" mm");
    grid_job->addWidget(spin_tail_strip_, 2, 1);

    grid_job->addWidget(new QLabel("Blade Strip Depth (mm):", this), 3, 0);
    spin_blade_depth_ = new QDoubleSpinBox(this);
    spin_blade_depth_->setRange(0.5, 4.0);
    spin_blade_depth_->setValue(3.0);
    spin_blade_depth_->setDecimals(2);
    spin_blade_depth_->setSuffix(" mm");
    grid_job->addWidget(spin_blade_depth_, 3, 1);

    grid_job->addWidget(new QLabel("Batch Target Quantity:", this), 4, 0);
    spin_quantity_ = new QSpinBox(this);
    spin_quantity_->setRange(1, 1000000);
    spin_quantity_->setValue(500);
    spin_quantity_->setSuffix(" pcs");
    grid_job->addWidget(spin_quantity_, 4, 1);

    mid_layout->addWidget(grp_job, 1);

    // Right Box: Operational Control & Live KPI Dashboard
    auto* grp_control = new QGroupBox("Reference Telemetry — No Production Execution", this);
    auto* v_ctrl = new QVBoxLayout(grp_control);

    auto* kpi_layout = new QHBoxLayout();
    lbl_pieces_count_ = new QLabel("Parts: NOT MODELED", this);
    lbl_pieces_count_->setObjectName("KpiValue");
    lbl_cycle_time_ = new QLabel("Cycle: NOT MODELED", this);
    lbl_throughput_ = new QLabel("Rate: NOT MODELED", this);

    kpi_layout->addWidget(lbl_pieces_count_);
    kpi_layout->addWidget(lbl_cycle_time_);
    kpi_layout->addWidget(lbl_throughput_);
    v_ctrl->addLayout(kpi_layout);

    progress_bar_ = new QProgressBar(this);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    v_ctrl->addWidget(progress_bar_);

    // Buttons Row
    auto* btn_layout = new QHBoxLayout();
    btn_start_ = new QPushButton("▶ START BATCH", this);
    btn_start_->setObjectName("BtnStart");
    connect(btn_start_, &QPushButton::clicked, this, &MainWindow::on_start_job_clicked);

    btn_pause_ = new QPushButton("⏸ PAUSE", this);
    btn_pause_->setObjectName("BtnPause");
    connect(btn_pause_, &QPushButton::clicked, this, &MainWindow::on_pause_job_clicked);

    btn_abort_ = new QPushButton("ABORT REQUEST", this);
    btn_abort_->setObjectName("BtnAbort");
    btn_abort_->setToolTip("Software request only. NOT a hardware emergency stop.");
    connect(btn_abort_, &QPushButton::clicked, this, &MainWindow::on_abort_job_clicked);

    btn_home_ = new QPushButton("⌂ HOME JAWS", this);
    btn_home_->setObjectName("BtnHome");
    connect(btn_home_, &QPushButton::clicked, this, &MainWindow::on_home_clicked);

    btn_door_ = new QPushButton("🔓 UNLOCK GUARD", this);
    btn_door_->setObjectName("BtnDoor");
    connect(btn_door_, &QPushButton::clicked, this, &MainWindow::on_unlock_door_clicked);

    btn_air_ = new QPushButton("💨 AIR JET", this);
    btn_air_->setObjectName("BtnAir");
    connect(btn_air_, &QPushButton::clicked, this, &MainWindow::on_air_blast_clicked);

    btn_layout->addWidget(btn_start_);
    btn_layout->addWidget(btn_pause_);
    btn_layout->addWidget(btn_abort_);
    btn_layout->addWidget(btn_home_);
    btn_layout->addWidget(btn_door_);
    btn_layout->addWidget(btn_air_);
    v_ctrl->addLayout(btn_layout);

    mid_layout->addWidget(grp_control, 2);
    main_layout->addLayout(mid_layout);

    // Bottom Section: Real-Time Machine Wire Track Animation
    auto* grp_track = new QGroupBox("Reference Axis Display — Injected Feedback, Not Hardware Measurements", this);
    auto* v_track = new QVBoxLayout(grp_track);
    v_track->addWidget(telem_view_);
    main_layout->addWidget(grp_track);
}

void MainWindow::apply_styling() {
    // Starz Corporate Brand Theme: #0B0F14 (Deep Black), #E10600 (Star Red), #E5E7EB (Light Gray)
    setStyleSheet(R"(
        QWidget {
            background-color: #0B0F14;
            color: #E5E7EB;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            font-size: 13px;
        }
        QGroupBox {
            border: 1px solid #30363D;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
            font-weight: bold;
            color: #F3F4F6;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            color: #E10600;
        }
        QLabel#TitleLabel {
            font-size: 18px;
            font-weight: 800;
            color: #FFFFFF;
            letter-spacing: 0.5px;
        }
        QLabel#StatusBadge {
            background-color: #21262D;
            color: #9CA3AF;
            padding: 4px 10px;
            border-radius: 4px;
            font-weight: bold;
            font-size: 11px;
        }
        QLabel#SafetyBadge {
            background-color: #1F2937;
            color: #38BDF8;
            padding: 4px 10px;
            border-radius: 4px;
            font-weight: bold;
            font-size: 11px;
        }
        QLabel#KpiValue {
            font-size: 20px;
            font-weight: bold;
            color: #FFFFFF;
        }
        QPushButton {
            background-color: #21262D;
            border: 1px solid #30363D;
            border-radius: 4px;
            padding: 8px 14px;
            font-weight: bold;
            color: #F3F4F6;
        }
        QPushButton:hover {
            background-color: #30363D;
            border-color: #8B949E;
        }
        QPushButton#BtnStart {
            background-color: #15803D;
            border-color: #16A34A;
            color: #FFFFFF;
        }
        QPushButton#BtnStart:hover {
            background-color: #16A34A;
        }
        QPushButton#BtnPause {
            background-color: #B45309;
            border-color: #D97706;
            color: #FFFFFF;
        }
        QPushButton#BtnAbort {
            background-color: #B91C1C;
            border-color: #DC2626;
            color: #FFFFFF;
        }
        QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #161B22;
            border: 1px solid #30363D;
            border-radius: 4px;
            padding: 6px;
            color: #FFFFFF;
        }
        QProgressBar {
            background-color: #161B22;
            border: 1px solid #30363D;
            border-radius: 4px;
            text-align: center;
            color: #FFFFFF;
            height: 18px;
        }
        QProgressBar::chunk {
            background-color: #E10600;
            border-radius: 3px;
        }
    )");
}

void MainWindow::refresh_serial_ports() {
    port_combo_->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& port : ports) {
        port_combo_->addItem(port.portName());
    }
}

void MainWindow::on_connect_clicked() {
    if (comm_->is_connected()) {
        comm_->disconnect_port();
    } else {
        QString port = port_combo_->currentText();
        if (!port.isEmpty()) {
            comm_->connect_to_port(port, 115200);
        }
    }
}

void MainWindow::on_refresh_ports_clicked() {
    refresh_serial_ports();
}

void MainWindow::on_start_job_clicked() {
    int32_t len_um = static_cast<int32_t>(spin_length_->value() * 1000.0);
    int32_t lead_um = static_cast<int32_t>(spin_lead_strip_->value() * 1000.0);
    int32_t tail_um = static_cast<int32_t>(spin_tail_strip_->value() * 1000.0);
    int32_t depth_um = static_cast<int32_t>(spin_blade_depth_->value() * 1000.0);
    uint32_t qty = static_cast<uint32_t>(spin_quantity_->value());

    comm_->send_job_start(len_um, lead_um, tail_um, depth_um, qty);
}

void MainWindow::on_pause_job_clicked() {
    comm_->send_job_pause();
}

void MainWindow::on_abort_job_clicked() {
    comm_->send_job_abort();
}

void MainWindow::on_home_clicked() {
    comm_->send_home();
}

void MainWindow::on_unlock_door_clicked() {
    comm_->send_solenoid_door();
}

void MainWindow::on_air_blast_clicked() {
    comm_->send_solenoid_air(80); // 80 ms air pulse
}

void MainWindow::on_connection_status(bool connected, const QString& message) {
    if (connected) {
        btn_connect_->setText("Disconnect");
        lbl_status_->setText("LINK OPEN / TELEMETRY UNKNOWN");
        lbl_status_->setStyleSheet("background-color: #92400E; color: #FFFFFF;");
    } else {
        btn_connect_->setText("Connect");
        lbl_status_->setText("DISCONNECTED");
        lbl_status_->setStyleSheet("background-color: #21262D; color: #9CA3AF;");
    }
    on_telemetry_validity_changed(false);
    statusBar()->showMessage(message, 5000);
}

void MainWindow::update_control_availability() {
    // Fresh telemetry is necessary but cannot commission an unimplemented process.
    for (auto* button : {btn_start_, btn_pause_, btn_home_, btn_door_, btn_air_}) {
        button->setEnabled(false);
        button->setToolTip("Disabled: reference prototype, NOT FOR MACHINE OPERATION");
    }
    btn_abort_->setEnabled(comm_->is_connected());
}

void MainWindow::on_telemetry_validity_changed(bool fresh) {
    update_control_availability();
    if (fresh) return;
    lbl_status_->setText(comm_->is_connected() ? "TELEMETRY STALE / UNKNOWN" : "DISCONNECTED");
    lbl_status_->setStyleSheet("background-color: #92400E; color: #FFFFFF;");
    lbl_safety_state_->setText("INPUTS: UNKNOWN");
    lbl_safety_state_->setStyleSheet("background-color: #7F1D1D; color: #FECACA;");
    lbl_pieces_count_->setText("Parts: UNKNOWN / NOT MODELED");
    lbl_cycle_time_->setText("Cycle: NOT MODELED");
    lbl_throughput_->setText("Rate: NOT MODELED");
    progress_bar_->setValue(0);
    telem_view_->invalidate_telemetry();
}

void MainWindow::on_telemetry_updated(const protocol::TelemetryPayload& telem) {
    if (!comm_->has_fresh_telemetry() || !TelemetryStatus::valid_reference_telemetry(telem)) {
        on_telemetry_validity_changed(false);
        return;
    }
    telem_view_->update_telemetry(telem);
    update_control_availability();

    // Update status badge
    switch (static_cast<protocol::MachineState>(telem.state)) {
        case protocol::MachineState::IDLE:
            lbl_status_->setText("STATE: IDLE");
            lbl_status_->setStyleSheet("background-color: #1E3A8A; color: #FFFFFF;");
            break;
        case protocol::MachineState::HOMING:
            lbl_status_->setText("STATE: HOMING");
            lbl_status_->setStyleSheet("background-color: #92400E; color: #FFFFFF;");
            break;
        case protocol::MachineState::RUNNING:
            lbl_status_->setText("REFERENCE AXIS TEST");
            lbl_status_->setStyleSheet("background-color: #92400E; color: #FFFFFF;");
            break;
        case protocol::MachineState::PAUSED:
            lbl_status_->setText("STATE: PAUSED");
            lbl_status_->setStyleSheet("background-color: #B45309; color: #FFFFFF;");
            break;
        case protocol::MachineState::FAULT:
            lbl_status_->setText("STATE: FAULT");
            lbl_status_->setStyleSheet("background-color: #B91C1C; color: #FFFFFF;");
            break;
        case protocol::MachineState::ESTOP:
            lbl_status_->setText("REPORTED E-STOP FAULT");
            lbl_status_->setStyleSheet("background-color: #DC2626; color: #FFFFFF;");
            break;
        default:
            lbl_status_->setText("STATE: INITIALIZING / UNKNOWN");
            lbl_status_->setStyleSheet("background-color: #92400E; color: #FFFFFF;");
            break;
    }

    // Update Safety Badge
    bool guard_locked = (telem.safety_flags & (1 << 1)) != 0;
    bool relay_ok = (telem.safety_flags & (1 << 0)) != 0;
    const bool inputs_fresh = (telem.safety_flags & protocol::INPUTS_FRESH_FLAG) != 0;
    if (!inputs_fresh) {
        lbl_safety_state_->setText("INPUTS: UNKNOWN / STALE");
        lbl_safety_state_->setStyleSheet("background-color: #7F1D1D; color: #FECACA;");
    } else if (telem.error_code != 0 || (telem.safety_flags & 0x30U) != 0) {
        lbl_safety_state_->setText(QString("REFERENCE FAULT: 0x%1").arg(telem.error_code, 4, 16, QChar('0')));
        lbl_safety_state_->setStyleSheet("background-color: #7F1D1D; color: #FECACA;");
    } else if (relay_ok && guard_locked && (telem.safety_flags & 0x0CU) == 0x0CU) {
        lbl_safety_state_->setText("REFERENCE INPUTS PRESENT — NOT SAFETY APPROVAL");
        lbl_safety_state_->setStyleSheet("background-color: #1E3A8A; color: #FFFFFF;");
    } else if (!guard_locked) {
        lbl_safety_state_->setText("REPORTED GUARD OPEN");
        lbl_safety_state_->setStyleSheet("background-color: #9A3412; color: #FED7AA;");
    } else {
        lbl_safety_state_->setText("REFERENCE INPUTS NOT READY");
        lbl_safety_state_->setStyleSheet("background-color: #7F1D1D; color: #FECACA;");
    }

    // Update KPIs
    lbl_pieces_count_->setText("Parts: NOT MODELED (0 accepted)");
    lbl_cycle_time_->setText("Cycle: NOT MODELED");
    lbl_throughput_->setText("Rate: NOT MODELED");
    progress_bar_->setValue(TelemetryStatus::progress_percent(telem.completed_pcs, telem.target_pcs));
}

} // namespace starz::hmi
