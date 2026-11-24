#include "oblivion/protocol/frame_codec.hpp"
#include "oblivion/simulation/autonomous_track_runner.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QAbstractItemView>
#include <deque>
#include <algorithm>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QByteArray>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

class SignalPlot final : public QWidget {
public:
 explicit SignalPlot(QWidget* p=nullptr):QWidget(p){setMinimumHeight(180);}
 void append(double v){samples_.push_back(v);while(samples_.size()>240)samples_.pop_front();update();}
protected:
 void paintEvent(QPaintEvent*) override {
  QPainter p(this);p.fillRect(rect(),QColor("#fbfbfc"));QRectF a=rect().adjusted(42,26,-12,-25);
  p.setPen(QColor("#737985"));p.drawText(12,18,"RAW PAYLOAD BYTE TRACE");
  p.setPen(QColor("#e4e6eb"));for(int i=0;i<=4;++i){qreal y=a.top()+a.height()*i/4.0;p.drawLine(QPointF(a.left(),y),QPointF(a.right(),y));}
  if(samples_.size()<2){p.setPen(QColor("#888"));p.drawText(a,Qt::AlignCenter,"Waiting for incoming packet payloads");return;}
  auto mm=std::minmax_element(samples_.begin(),samples_.end());double lo=*mm.first,hi=*mm.second;if(hi-lo<1){lo-=1;hi+=1;}
  QPolygonF line;for(std::size_t i=0;i<samples_.size();++i){qreal x=a.left()+a.width()*i/(samples_.size()-1);qreal y=a.bottom()-(samples_[i]-lo)/(hi-lo)*a.height();line<<QPointF(x,y);}
  p.setPen(QPen(QColor("#3478d4"),2));p.drawPolyline(line);
  p.setPen(QColor("#737985"));p.drawText(QRectF(2,a.top(),36,16),Qt::AlignRight,QString::number(hi,'f',1));p.drawText(QRectF(2,a.bottom()-16,36,16),Qt::AlignRight,QString::number(lo,'f',1));
 }
private:std::deque<double> samples_;
};

class OblivionWindow final : public QMainWindow {
public:
    OblivionWindow() {
        setWindowTitle("OBLIVION Control Center");
        resize(1420, 900);
        setMinimumSize(1120, 760);
        setStyleSheet(R"QSS(
            QMainWindow, QWidget#root { background:#f5f5f7; color:#202124; font-family:"Segoe UI"; font-size:13px; }
            QLabel#eyebrow { color:#777b84; font-size:10px; font-weight:700; letter-spacing:2px; }
            QLabel#brand { color:#17191d; font-size:27px; font-weight:800; letter-spacing:3px; }
            QLabel#subtitle { color:#737780; font-size:12px; }
            QFrame#surface { background:#ffffff; border:1px solid #e6e7eb; border-radius:14px; }
            QLabel#sectionTitle { color:#24262b; font-size:14px; font-weight:700; }
            QLabel#muted { color:#858993; font-size:11px; }
            QLabel#metric { color:#1e2025; font-size:20px; font-weight:700; }
            QLabel#statusConnected { color:#16865b; background:#e8f6ef; border:1px solid #ccebd9; border-radius:9px; padding:7px 12px; font-weight:700; }
            QLabel#statusDisconnected { color:#a66a16; background:#fff5e5; border:1px solid #f3dfb9; border-radius:9px; padding:7px 12px; font-weight:700; }
            QPushButton { background:#f3f4f6; color:#25272c; border:1px solid #e0e2e7; border-radius:9px; padding:9px 14px; font-weight:600; }
            QPushButton:hover { background:#e9ebef; }
            QPushButton:disabled { color:#aeb1b8; background:#f6f6f7; }
            QPushButton#primary { background:#202329; color:white; border:1px solid #202329; }
            QPushButton#primary:hover { background:#363a43; }
            QPushButton#danger { background:#fff0ef; color:#bd3434; border:1px solid #f2cecb; }
            QPushButton#danger:hover { background:#ffe2df; }
            QPushButton#profile { background:#f6f7f9; border:1px solid #e4e6eb; padding:13px 18px; text-align:left; }
            QPushButton#profile:checked { background:#edf3ff; color:#2457a7; border:1px solid #9db9ed; }
            QComboBox { background:#fff; border:1px solid #dfe1e6; border-radius:8px; padding:8px 10px; min-height:18px; }
            QComboBox::drop-down { border:0; width:24px; }
            QCheckBox { color:#34363c; spacing:8px; padding:5px; }
            QCheckBox::indicator { width:16px; height:16px; }
            QTextEdit { background:#fbfbfc; border:1px solid #e7e8ec; border-radius:10px; padding:9px; color:#34363c; font-family:Consolas; font-size:11px; }
            QScrollArea { border:0; background:transparent; }
            QStatusBar { background:#fff; color:#777b84; border-top:1px solid #e7e8ec; }
        )QSS");

        auto* root = new QWidget;
        root->setObjectName("root");
        auto* outer = new QVBoxLayout(root);
        outer->setContentsMargins(26, 22, 26, 16);
        outer->setSpacing(16);

        auto* header = new QHBoxLayout;
        auto* identity = new QVBoxLayout;
        auto* eyebrow = new QLabel("DMJ  /  AUTONOMOUS SYSTEMS");
        eyebrow->setObjectName("eyebrow");
        auto* brand = new QLabel("OBLIVION");
        brand->setObjectName("brand");
        auto* subtitle = new QLabel("Robotics Control Center");
        subtitle->setObjectName("subtitle");
        identity->addWidget(eyebrow);
        identity->addWidget(brand);
        identity->addWidget(subtitle);
        header->addLayout(identity);
        header->addStretch();
        status_ = new QLabel("●  OFFLINE");
        status_->setObjectName("statusDisconnected");
        header->addWidget(status_, 0, Qt::AlignVCenter);
        outer->addLayout(header);

        auto* connection = surface("ROBOT CONNECTION");
        auto* conn = new QHBoxLayout(connection);
        conn->setContentsMargins(16, 14, 16, 14);
        port_ = new QComboBox;
        port_->setMinimumWidth(320);
        refresh_ = new QPushButton("Refresh");
        connect_ = new QPushButton("Connect");
        connect_->setObjectName("primary");
        disconnect_ = new QPushButton("Disconnect");
        disconnect_->setEnabled(false);
        conn->addWidget(new QLabel("Serial interface"));
        conn->addWidget(port_, 1);
        conn->addWidget(refresh_);
        conn->addWidget(connect_);
        conn->addWidget(disconnect_);
        outer->addWidget(connection);

        auto* profileCard = surface("MISSION PROFILE");
        auto* profileLayout = new QVBoxLayout(profileCard);
        profileLayout->setContentsMargins(16, 14, 16, 16);
        auto* profileRow = new QHBoxLayout;
        sprint_ = new QPushButton("SPRINT   ·   High-speed line following");
        explore_ = new QPushButton("EXPLORE   ·   Maze mapping & route solving");
        sprint_->setObjectName("profile");
        explore_->setObjectName("profile");
        sprint_->setCheckable(true);
        explore_->setCheckable(true);
        sprint_->setChecked(true);
        profileRow->addWidget(sprint_, 1);
        profileRow->addWidget(explore_, 1);
        profileLayout->addLayout(profileRow);
        outer->addWidget(profileCard);

        auto* controllerCard = surface("PARALLEL CONTROLLER ENSEMBLE");
        auto* controllerLayout = new QVBoxLayout(controllerCard);
        controllerLayout->setContentsMargins(16, 14, 16, 16);
        auto* controllerHeader = new QHBoxLayout;
        auto* controllerHint = new QLabel("Select multiple controllers. The onboard supervisor evaluates the enabled ensemble and applies its validated arbitration policy.");
        controllerHint->setObjectName("muted");
        controllerHint->setWordWrap(true);
        controllerHeader->addWidget(controllerHint, 1);
        selectedCount_ = new QLabel("0 SELECTED");
        selectedCount_->setObjectName("eyebrow");
        controllerHeader->addWidget(selectedCount_);
        controllerLayout->addLayout(controllerHeader);

        const std::array<const char*, 11> names = {"PID","LQR","MPC","Sliding Mode","H-infinity","ADRC","Backstepping","Adaptive PD","Gain Scheduled","Fuzzy","Robust Feedback"};
        const std::array<const char*, 11> descriptions = {
            "Baseline tracking and trim","Optimal state feedback","Predictive constrained control",
            "Robust nonlinear tracking","Disturbance rejection bounds","Active disturbance estimation",
            "Nonlinear dynamics control","Online parameter adaptation","Operating-point scheduling",
            "Rule-based nonlinear fusion","Validated fallback controller"
        };
        auto* grid = new QGridLayout;
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(5);
        for (std::size_t i=0;i<names.size();++i) {
            auto* item = new QWidget;
            auto* itemLayout = new QVBoxLayout(item);
            itemLayout->setContentsMargins(5, 3, 5, 3);
            auto* check = new QCheckBox(names[i]);
            auto* desc = new QLabel(descriptions[i]);
            desc->setObjectName("muted");
            desc->setContentsMargins(25,0,0,0);
            itemLayout->addWidget(check);
            itemLayout->addWidget(desc);
            grid->addWidget(item, static_cast<int>(i/3), static_cast<int>(i%3));
            controllers_.push_back(check);
            connect(check, &QCheckBox::toggled, this, [this]{ update_controller_count(); });
        }
        controllerLayout->addLayout(grid);
        auto* selectionActions = new QHBoxLayout;
        auto* selectRecommended = new QPushButton("Select recommended ensemble");
        auto* clearSelection = new QPushButton("Clear selection");
        selectionActions->addWidget(selectRecommended);
        selectionActions->addWidget(clearSelection);
        selectionActions->addStretch();
        controllerLayout->addLayout(selectionActions);
        outer->addWidget(controllerCard);

        auto* actionRow = new QHBoxLayout;
        start_ = new QPushButton("Start mission"); start_->setObjectName("primary");
        pause_ = new QPushButton("Pause"); stop_ = new QPushButton("Stop"); estop_ = new QPushButton("Emergency stop"); estop_->setObjectName("danger");
        start_->setEnabled(false); pause_->setEnabled(false); stop_->setEnabled(false); estop_->setEnabled(false);
        actionRow->addWidget(start_); actionRow->addWidget(pause_); actionRow->addWidget(stop_); actionRow->addWidget(estop_);
        actionRow->addStretch();
        auto* calibrate = new QPushButton("Calibrate sensors");
        auto* heartbeat = new QPushButton("Heartbeat");
        auto* simulate = new QPushButton("Run local simulation");
        actionRow->addWidget(calibrate); actionRow->addWidget(heartbeat); actionRow->addWidget(simulate);
        outer->addLayout(actionRow);

        auto* lower = new QHBoxLayout;
        auto* telemetryCard = surface("EVENT & TELEMETRY CONSOLE");
        auto* telemetryLayout = new QVBoxLayout(telemetryCard);
        telemetryLayout->setContentsMargins(14, 12, 14, 12);
        auto* telemetryHeader = new QHBoxLayout;
        auto* telemetryHint = new QLabel("Connection events, protocol frames and simulation summaries");
        telemetryHint->setObjectName("muted");
        auto* clear = new QPushButton("Clear");
        telemetryHeader->addWidget(telemetryHint,1);
        telemetryHeader->addWidget(clear);
        telemetryLayout->addLayout(telemetryHeader);
        telemetry_ = new QTextEdit;
        telemetry_->setReadOnly(true);
        telemetry_->setMinimumHeight(160);
        telemetry_->setPlaceholderText("Waiting for robot connection or local simulation...");
        telemetryLayout->addWidget(telemetry_);
        lower->addWidget(telemetryCard, 3);

        auto* safetyCard = surface("SAFETY INTERLOCK");
        auto* safetyLayout = new QVBoxLayout(safetyCard);
        safetyLayout->setContentsMargins(16, 14, 16, 14);
        auto* safety = new QLabel("Emergency stop is a request to firmware. A disconnected link does not guarantee motor shutdown. Firmware must enforce watchdog timeouts, motor-off startup defaults and an independent hardware-safe E-stop.");
        safety->setWordWrap(true);
        safety->setObjectName("muted");
        safetyLayout->addWidget(safety);
        lower->addWidget(safetyCard, 1);
        outer->addLayout(lower, 1);

        setCentralWidget(root);
        statusBar()->showMessage("Ready · No robot connected");
        serial_ = std::make_unique<QSerialPort>();
        refresh_ports();
        connect(refresh_, &QPushButton::clicked, this, [this]{ refresh_ports(); });
        connect(connect_, &QPushButton::clicked, this, [this]{ open_port(); });
        connect(disconnect_, &QPushButton::clicked, this, [this]{ close_port(); });
        connect(sprint_, &QPushButton::clicked, this, [this]{ sprint_->setChecked(true); explore_->setChecked(false); });
        connect(explore_, &QPushButton::clicked, this, [this]{ explore_->setChecked(true); sprint_->setChecked(false); });
        connect(selectRecommended, &QPushButton::clicked, this, [this]{
            for (std::size_t i=0;i<controllers_.size();++i) controllers_[i]->setChecked(i==0 || i==1 || i==2 || i==4 || i==5 || i==9);
        });
        connect(clearSelection, &QPushButton::clicked, this, [this]{ for(auto* c:controllers_) c->setChecked(false); });
        connect(start_, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::MotorCommand,1); });
        connect(pause_, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::MotorCommand,2); });
        connect(stop_, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::MotorCommand,0); });
        connect(estop_, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::EmergencyStop,0); });
        connect(calibrate, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::Calibration,0x10); });
        connect(heartbeat, &QPushButton::clicked, this, [this]{ send_action(oblivion::protocol::MessageType::Heartbeat,0); });
        connect(simulate, &QPushButton::clicked, this, [this]{ run_local_simulation(); });
        connect(clear, &QPushButton::clicked, telemetry_, &QTextEdit::clear);
        connect(serial_.get(), &QSerialPort::readyRead, this, [this]{ read_serial(); });
        connect(serial_.get(), &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError e){ if(e!=QSerialPort::NoError) log("Serial error: "+serial_->errorString()); });
        refresh_timer_.setInterval(2000);
        connect(&refresh_timer_, &QTimer::timeout, this, [this]{ if(!serial_->isOpen()) refresh_ports(); });
        refresh_timer_.start();
        update_controller_count();
    }

    ~OblivionWindow() override { if(serial_ && serial_->isOpen()) serial_->close(); }

private:
    QFrame* surface(const QString& title) {
        auto* frame = new QFrame;
        frame->setObjectName("surface");
        auto* layout = new QVBoxLayout(frame);
        auto* heading = new QLabel(title);
        heading->setObjectName("sectionTitle");
        layout->addWidget(heading);
        return frame;
    }
    void update_controller_count() {
        int count=0; for(auto* c:controllers_) if(c->isChecked()) ++count;
        selectedCount_->setText(QString::number(count)+" SELECTED");
        start_->setEnabled(serial_->isOpen() && count>0);
    }
    std::uint16_t controller_mask() const {
        std::uint16_t mask=0;
        for(std::size_t i=0;i<controllers_.size();++i) if(controllers_[i]->isChecked()) mask|=static_cast<std::uint16_t>(1u<<i);
        return mask;
    }
    void log(const QString& s) { telemetry_->append("["+QDateTime::currentDateTime().toString("HH:mm:ss.zzz")+"]  "+s); }
    void refresh_ports() {
        const QString previous=port_->currentData().toString(); port_->clear();
        for(const auto& p:QSerialPortInfo::availablePorts()) port_->addItem(p.portName()+"  ·  "+p.description(),p.portName());
        const int i=port_->findData(previous); if(i>=0) port_->setCurrentIndex(i);
    }
    void open_port() {
        if(port_->currentData().toString().isEmpty()){QMessageBox::warning(this,"No serial interface","Connect the robot controller and select its serial interface.");return;}
        serial_->setPortName(port_->currentData().toString()); serial_->setBaudRate(QSerialPort::Baud115200);
        serial_->setDataBits(QSerialPort::Data8); serial_->setParity(QSerialPort::NoParity); serial_->setStopBits(QSerialPort::OneStop); serial_->setFlowControl(QSerialPort::NoFlowControl);
        if(!serial_->open(QIODevice::ReadWrite)){QMessageBox::critical(this,"Connection failed",serial_->errorString());return;}
        status_->setText("●  LINK OPEN"); status_->setObjectName("statusConnected"); status_->style()->unpolish(status_); status_->style()->polish(status_);
        connect_->setEnabled(false); disconnect_->setEnabled(true); port_->setEnabled(false);
        pause_->setEnabled(true); stop_->setEnabled(true); estop_->setEnabled(true); update_controller_count();
        statusBar()->showMessage("Serial link open · 115200 baud · Firmware identity not yet verified");
        log("Serial link opened on "+serial_->portName()+" at 115200 baud.");
        send_action(oblivion::protocol::MessageType::Heartbeat,0);
    }
    void close_port() {
        if(serial_->isOpen()) serial_->close();
        status_->setText("●  OFFLINE"); status_->setObjectName("statusDisconnected"); status_->style()->unpolish(status_); status_->style()->polish(status_);
        connect_->setEnabled(true); disconnect_->setEnabled(false); port_->setEnabled(true);
        pause_->setEnabled(false); stop_->setEnabled(false); estop_->setEnabled(false); update_controller_count();
        statusBar()->showMessage("Disconnected · Firmware watchdog must independently stop the motors");
        log("Serial link closed. Firmware watchdog must independently stop motors.");
    }
    void send_action(oblivion::protocol::MessageType type,std::uint8_t action) {
        if(!serial_->isOpen()){log("Rejected: no robot connection.");return;}
        const std::uint16_t mask=controller_mask();
        if(type==oblivion::protocol::MessageType::MotorCommand && action==1 && mask==0){log("Start rejected: select at least one controller.");return;}
        oblivion::protocol::Packet packet{}; packet.type=type; packet.sequence=++sequence_;
        packet.payload={action,static_cast<std::uint8_t>(sprint_->isChecked()?0:1),static_cast<std::uint8_t>(mask&0xffu),static_cast<std::uint8_t>((mask>>8u)&0xffu)};
        const auto bytes=oblivion::protocol::encode(packet);
        const auto written=serial_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size()));
        if(written!=static_cast<qint64>(bytes.size())){log("Transport write failed: "+serial_->errorString());return;}
        const QString name=type==oblivion::protocol::MessageType::EmergencyStop?"EMERGENCY STOP":type==oblivion::protocol::MessageType::Heartbeat?"HEARTBEAT":type==oblivion::protocol::MessageType::Calibration?"CALIBRATION":action==1?"START REQUEST":action==2?"PAUSE REQUEST":"STOP REQUEST";
        log("TX "+name+" · seq "+QString::number(sequence_)+" · profile "+(sprint_->isChecked()?"Sprint":"Explore")+" · controller mask 0x"+QString::number(mask,16).toUpper());
    }
    void run_local_simulation() {
        oblivion::TrackBuilder builder(0.012);
        if(!builder.add_straight({0.0,0.0},{1.0,0.0})){log("Simulation setup failed: track generation.");return;}
        oblivion::TrackRunnerConfig cfg{};
        cfg.plant.maximum_wheel_speed_mps=1.5; cfg.plant.track_width_m=0.12; cfg.plant.wheel_radius_m=0.02;
        cfg.motion.maximum_speed_mps=1.0; cfg.motion.maximum_wheel_speed_mps=1.5; cfg.motion.wheel_track_m=0.12;
        cfg.simulated_sensors.sensor_count=8; cfg.perception.sensor_count=8;
        cfg.perception.line_threshold=0.5; cfg.perception.minimum_total_strength=0.2; cfg.perception.line_is_dark=true;
        for(std::size_t i=0;i<8;++i){const double p=(static_cast<double>(i)-3.5)*0.01;cfg.simulated_sensors.lateral_positions_m[i]=p;cfg.perception.positions_m[i]=p;cfg.perception.dark_level[i]=0.0;cfg.perception.light_level[i]=1.0;}
        oblivion::AutonomousTrackRunner runner(cfg);
        if(!runner.configuration_valid()){log("Simulation setup failed: invalid runner configuration.");return;}
        const auto metrics=runner.run(builder.build(),{},0.005,6000);
        if(!metrics.valid){log("Simulation failed or produced no valid run metrics.");QMessageBox::warning(this,"Simulation failed","The local track simulation did not produce valid metrics.");return;}
        const QString report=QString("Completed: %1 · Time: %2 s · RMS error: %3 mm · Max error: %4 mm · Steps: %5")
            .arg(metrics.completed?"yes":"no").arg(metrics.simulated_time_seconds,0,'f',3)
            .arg(metrics.rms_cross_track_error_m*1000.0,0,'f',2).arg(metrics.maximum_cross_track_error_m*1000.0,0,'f',2)
            .arg(static_cast<qulonglong>(metrics.steps));
        log("LOCAL SIMULATION  "+report); QMessageBox::information(this,"OBLIVION simulation",report);
    }
    void read_serial() {
        receive_buffer_.append(serial_->readAll());
        while(receive_buffer_.size()>=12) {
            const auto* data=reinterpret_cast<const std::uint8_t*>(receive_buffer_.constData());
            if(data[0]!=oblivion::protocol::kMagic0||data[1]!=oblivion::protocol::kMagic1){receive_buffer_.remove(0,1);continue;}
            const int payload=static_cast<int>(data[8])|(static_cast<int>(data[9])<<8); const int frameSize=12+payload;
            if(payload>512){receive_buffer_.remove(0,1);continue;} if(receive_buffer_.size()<frameSize)break;
            oblivion::protocol::Packet packet{};
            if(oblivion::protocol::decode(data,static_cast<std::size_t>(frameSize),packet))
                log("RX frame type "+QString::number(static_cast<int>(packet.type))+" · seq "+QString::number(packet.sequence)+" · "+QString::number(packet.payload.size())+" bytes");
            else log("RX invalid frame rejected (CRC/version/length).");
            receive_buffer_.remove(0,frameSize);
        }
    }
    QComboBox* port_{}; QLabel* status_{}; QLabel* selectedCount_{}; QTextEdit* telemetry_{};
    QPushButton* refresh_{}; QPushButton* connect_{}; QPushButton* disconnect_{}; QPushButton* sprint_{}; QPushButton* explore_{};
    QPushButton* start_{}; QPushButton* pause_{}; QPushButton* stop_{}; QPushButton* estop_{};
    std::vector<QCheckBox*> controllers_;
    std::unique_ptr<QSerialPort> serial_; QTimer refresh_timer_; QByteArray receive_buffer_; std::uint32_t sequence_{0};
};

int main(int argc,char** argv) {
    QApplication app(argc,argv);
    QApplication::setApplicationName("OBLIVION Control Center");
    QApplication::setOrganizationName("DMJ");
    OblivionWindow window; window.show(); return app.exec();
}
