#include "oblivion/protocol/frame_codec.hpp"
#include "oblivion/protocol/telemetry.hpp"
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
#include <QTcpSocket>
#include <QLineEdit>
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
            QMainWindow, QWidget#root { background:#090d14; color:#e8edf5; font-family:"Segoe UI"; font-size:13px; }
            QLabel#eyebrow { color:#91a4bd; font-size:10px; font-weight:700; letter-spacing:2px; }
            QLabel#brand { color:#f5f7fb; font-size:27px; font-weight:800; letter-spacing:3px; }
            QLabel#subtitle { color:#9aabc0; font-size:12px; }
            QFrame#surface { background:#111925; border:1px solid #243247; border-radius:16px; }
            QLabel#sectionTitle { color:#e8edf5; font-size:14px; font-weight:700; }
            QLabel#muted { color:#9aabc0; font-size:11px; }
            QLabel#metric { color:#f5f7fb; font-size:20px; font-weight:700; }
            QLabel#statusConnected { color:#16865b; background:#e8f6ef; border:1px solid #ccebd9; border-radius:9px; padding:7px 12px; font-weight:700; }
            QLabel#statusDisconnected { color:#a66a16; background:#fff5e5; border:1px solid #f3dfb9; border-radius:9px; padding:7px 12px; font-weight:700; }
            QPushButton { background:#172235; color:#e5edf8; border:1px solid #2a3b54; border-radius:10px; padding:10px 15px; font-weight:600; }
            QPushButton:hover { background:#22324a; border-color:#5276a5; }
            QPushButton:disabled { color:#64748b; background:#111925; }
            QPushButton#primary { background:#1c6b67; color:#ffffff; border:1px solid #2a9a91; }
            QPushButton#primary:hover { background:#25857e; }
            QPushButton#danger { background:#3a1b24; color:#ff9da8; border:1px solid #713342; }
            QPushButton#danger:hover { background:#52232f; }
            QPushButton#profile { background:#131e2c; color:#c7d4e5; border:1px solid #293b53; padding:16px 18px; text-align:left; }
            QPushButton#profile:checked { background:#152f35; color:#83e5d9; border:1px solid #328b83; }
            QComboBox, QLineEdit { background:#0c1420; color:#e5edf8; border:1px solid #2a3b54; border-radius:8px; padding:8px 10px; min-height:18px; }
            QComboBox::drop-down { border:0; width:24px; }
            QCheckBox { color:#dce6f3; spacing:8px; padding:5px; }
            QCheckBox::indicator { width:16px; height:16px; }
            QTextEdit, QTableWidget { background:#0c1420; border:1px solid #26364c; border-radius:10px; padding:9px; color:#dce6f3; font-family:Consolas; font-size:11px; }
            QScrollArea { border:0; background:transparent; }
            QStatusBar { background:#0b111b; color:#91a4bd; border-top:1px solid #243247; } QHeaderView::section { background:#172235; color:#9fb3cc; border:0; padding:7px; }
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
        transport_ = new QComboBox; transport_->addItems({"Physical robot · Serial","Standalone simulator · TCP"});
        host_ = new QLineEdit("127.0.0.1"); host_->setMaximumWidth(150);
        tcpPort_ = new QLineEdit("8765"); tcpPort_->setMaximumWidth(78);
        port_ = new QComboBox;
        port_->setMinimumWidth(320);
        refresh_ = new QPushButton("Refresh");
        connect_ = new QPushButton("Connect");
        connect_->setObjectName("primary");
        disconnect_ = new QPushButton("Disconnect");
        disconnect_->setEnabled(false);
        conn->addWidget(transport_);
        conn->addWidget(port_, 1);
        conn->addWidget(host_);
        conn->addWidget(tcpPort_);
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
        actionRow->addWidget(calibrate); actionRow->addWidget(heartbeat);
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

        auto* tabs = new QTabWidget;
        tabs->setDocumentMode(true);
        tabs->addTab(root,"Control Center");
        auto* monitor=new QWidget;
        auto* ml=new QVBoxLayout(monitor);
        ml->setContentsMargins(22,18,22,18);ml->setSpacing(12);
        auto* heading=new QLabel("LIVE SYSTEM MONITOR");heading->setObjectName("brand");ml->addWidget(heading);
        auto* cards=new QGridLayout;
        rxFrames_=metricCard(cards,0,0,"RECEIVED FRAMES","0");
        rxBytes_=metricCard(cards,0,1,"RECEIVED BYTES","0 B");
        rxRate_=metricCard(cards,0,2,"FRAME RATE","0 /s");
        rxErrors_=metricCard(cards,0,3,"INVALID FRAMES","0");
        ml->addLayout(cards);
        auto* plotCard=surface("RAW SIGNAL TRACE");auto* pl=new QVBoxLayout(plotCard);
        plot_=new SignalPlot;pl->addWidget(plot_);ml->addWidget(plotCard,1);
        auto* tables=new QHBoxLayout;
        auto* rawCard=surface("RAW PACKET INSPECTOR");auto* rl=new QVBoxLayout(rawCard);
        rawTable_=new QTableWidget(0,5);rawTable_->setHorizontalHeaderLabels({"Time","Type","Sequence","Length","Payload HEX"});
        rawTable_->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);
        rawTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);rawTable_->verticalHeader()->hide();rl->addWidget(rawTable_);
        tables->addWidget(rawCard,3);
        auto* decodedCard=surface("DECODED / DERIVED PARAMETERS");auto* dl=new QVBoxLayout(decodedCard);
        decodedTable_=new QTableWidget(0,3);decodedTable_->setHorizontalHeaderLabels({"Parameter","Value","Source"});
        decodedTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);decodedTable_->verticalHeader()->hide();
        decodedTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        const QStringList rows={"Line position|—|Awaiting typed sensor telemetry","Line confidence|—|Awaiting typed sensor telemetry","Encoder counts|—|Awaiting encoder telemetry","Linear velocity|—|Derived after encoder decode","Yaw rate|—|Awaiting IMU telemetry","Acceleration|—|Awaiting IMU telemetry","Curvature|—|Derived after state decode","Controller outputs|—|Awaiting controller telemetry","Execution time|—|Awaiting runtime telemetry","CPU / RAM|—|Not reported by current protocol","Deadline misses|—|Awaiting scheduler telemetry","Battery voltage|—|Awaiting power telemetry"};
        for(const auto& row:rows){const auto fields=row.split("|");int n=decodedTable_->rowCount();decodedTable_->insertRow(n);for(int j=0;j<3;++j)decodedTable_->setItem(n,j,new QTableWidgetItem(fields[j]));}
        dl->addWidget(decodedTable_);tables->addWidget(decodedCard,2);ml->addLayout(tables,1);
        tabs->addTab(monitor,"Live Monitor");
        setCentralWidget(tabs);
        statusBar()->showMessage("Ready · No robot connected");
        serial_ = std::make_unique<QSerialPort>();
        tcp_ = std::make_unique<QTcpSocket>();
        connect(tcp_.get(), &QTcpSocket::readyRead, this, [this]{ read_serial(); });
        connect(transport_, &QComboBox::currentIndexChanged, this, [this](int mode){ const bool tcp=mode==1; port_->setVisible(!tcp); refresh_->setVisible(!tcp); host_->setVisible(tcp); tcpPort_->setVisible(tcp); });
        host_->hide(); tcpPort_->hide();
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

        connect(clear, &QPushButton::clicked, telemetry_, &QTextEdit::clear);
        connect(serial_.get(), &QSerialPort::readyRead, this, [this]{ read_serial(); });
        connect(serial_.get(), &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError e){ if(e!=QSerialPort::NoError) log("Serial error: "+serial_->errorString()); });
        refresh_timer_.setInterval(2000);
        connect(&refresh_timer_, &QTimer::timeout, this, [this]{ if(!serial_->isOpen()) refresh_ports(); });
        refresh_timer_.start();
        update_controller_count();
        rateTimer_.setInterval(1000);
        connect(&rateTimer_,&QTimer::timeout,this,[this]{rxRate_->setText(QString::number(rate_)+" /s");rate_=0;});
        rateTimer_.start();
    }

    ~OblivionWindow() override { if(serial_ && serial_->isOpen()) serial_->close(); }

private:
    void set_parameter(int row,const QString& value,const QString& source){
        decodedTable_->item(row,1)->setText(value);decodedTable_->item(row,2)->setText(source);
    }
    void update_decoded_telemetry(const oblivion::protocol::TelemetryV1& t){
        using namespace oblivion::protocol;
        const auto has=[&](TelemetryValidity bit){return (t.validity&static_cast<std::uint32_t>(bit))!=0;};
        if(has(ValidLineEstimate)){
            set_parameter(0,QString::number(t.line_position_um/1000.0,'f',3)+" mm","Telemetry v1 · line estimate");
            set_parameter(1,QString::number(t.line_confidence_permille/10.0,'f',1)+" %","Telemetry v1 · confidence");
        }
        if(has(ValidEncoders))set_parameter(2,QString::number(t.left_encoder_ticks)+" / "+QString::number(t.right_encoder_ticks),"Telemetry v1 · encoder ticks");
        if(has(ValidMotion)){
            set_parameter(3,QString::number(t.linear_velocity_mmps/1000.0,'f',3)+" m/s","Telemetry v1 · reported velocity");
            set_parameter(4,QString::number(t.yaw_rate_mradps/1000.0,'f',3)+" rad/s","Telemetry v1 · reported yaw rate");
            set_parameter(6,QString::number(t.curvature_milli_inv_m/1000.0,'f',4)+" 1/m","Telemetry v1 · reported curvature");
        }
        if(has(ValidImu)){
            const QString a=QString("%1, %2, %3 m/s²").arg(t.accel_x_mg*9.80665/1000.0,0,'f',2).arg(t.accel_y_mg*9.80665/1000.0,0,'f',2).arg(t.accel_z_mg*9.80665/1000.0,0,'f',2);
            set_parameter(5,a,"Telemetry v1 · accelerometer");
        }
        if(has(ValidControllerStats)){
            QStringList active;for(std::size_t i=0;i<t.controllers.size();++i)if((t.active_controller_mask&(1u<<i))!=0)active<<QString("C%1: %2 / %3 µs").arg(i).arg(t.controllers[i].output_milli/1000.0,0,'f',3).arg(t.controllers[i].execution_time_us);
            set_parameter(7,active.isEmpty()?"No active controller slots":active.join(" · "),"Telemetry v1 · controller candidates");
            std::uint32_t maxUs=0;for(const auto& slot:t.controllers)if(slot.status==1)maxUs=std::max<std::uint32_t>(maxUs,slot.execution_time_us);
            set_parameter(8,QString::number(maxUs)+" µs (max valid slot)","Telemetry v1 · controller timing");
        }
        if(has(ValidSystemHealth))set_parameter(9,QString::number(t.cpu_load_percent)+" % CPU · "+QString::number(t.free_memory_bytes)+" B free","Telemetry v1 · target-reported");
        if(has(ValidPower))set_parameter(11,QString::number(t.battery_millivolts/1000.0,'f',3)+" V","Telemetry v1 · battery monitor");
        set_parameter(10,"Fault flags: 0x"+QString::number(t.fault_flags,16).toUpper(),"Telemetry v1 · fault bitfield");
    }
    QLabel* metricCard(QGridLayout* grid,int row,int col,const QString& title,const QString& value){
        auto* card=new QFrame;card->setObjectName("surface");auto* l=new QVBoxLayout(card);l->setContentsMargins(12,10,12,10);
        auto* t=new QLabel(title);t->setObjectName("eyebrow");auto* v=new QLabel(value);v->setObjectName("metric");l->addWidget(t);l->addWidget(v);grid->addWidget(card,row,col);return v;
    }
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
    bool connected() const { return serial_->isOpen() || (tcp_ && tcp_->state()==QAbstractSocket::ConnectedState); }
    void open_port() {
        if(transport_->currentIndex()==1){
            bool ok=false; const int p=tcpPort_->text().toInt(&ok);
            if(!ok||p<1||p>65535){QMessageBox::warning(this,"Invalid endpoint","Enter a valid TCP port.");return;}
            tcp_->connectToHost(host_->text().trimmed(),static_cast<quint16>(p));
            if(!tcp_->waitForConnected(2500)){QMessageBox::critical(this,"Simulator connection failed",tcp_->errorString());return;}
            log("Simulator TCP connected · "+host_->text()+":"+QString::number(p));
        }else{
            if(port_->currentData().toString().isEmpty()){QMessageBox::warning(this,"No serial interface","Connect the robot controller and select its serial interface.");return;}
            serial_->setPortName(port_->currentData().toString()); serial_->setBaudRate(QSerialPort::Baud115200);
            serial_->setDataBits(QSerialPort::Data8); serial_->setParity(QSerialPort::NoParity); serial_->setStopBits(QSerialPort::OneStop); serial_->setFlowControl(QSerialPort::NoFlowControl);
            if(!serial_->open(QIODevice::ReadWrite)){QMessageBox::critical(this,"Connection failed",serial_->errorString());return;}
            log("Physical serial link opened · "+serial_->portName());
        }
        status_->setText(transport_->currentIndex()==1?"●  SIMULATOR ONLINE":"●  ROBOT LINK OPEN");
        status_->setObjectName("statusConnected"); status_->style()->unpolish(status_); status_->style()->polish(status_);
        connect_->setEnabled(false); disconnect_->setEnabled(true); port_->setEnabled(false); transport_->setEnabled(false);
        pause_->setEnabled(true); stop_->setEnabled(true); estop_->setEnabled(true); update_controller_count();
        statusBar()->showMessage("Connected · protocol v1 · telemetry stream awaiting/active");
        send_action(oblivion::protocol::MessageType::Heartbeat,0);
    }
    void close_port() {
        if(serial_->isOpen()) serial_->close(); if(tcp_ && tcp_->state()!=QAbstractSocket::UnconnectedState) tcp_->disconnectFromHost();
        if(tcp_ && tcp_->state()!=QAbstractSocket::UnconnectedState) tcp_->disconnectFromHost();
        status_->setText("●  OFFLINE"); status_->setObjectName("statusDisconnected"); status_->style()->unpolish(status_); status_->style()->polish(status_);
        connect_->setEnabled(true); disconnect_->setEnabled(false); port_->setEnabled(true); transport_->setEnabled(true);
        pause_->setEnabled(false); stop_->setEnabled(false); estop_->setEnabled(false); update_controller_count();
        statusBar()->showMessage("Disconnected · physical firmware watchdog remains responsible for motor safety");
        log("Transport disconnected.");
    }
    void send_action(oblivion::protocol::MessageType type,std::uint8_t action) {
        if(!connected()){log("Rejected: no robot connection.");return;}
        const std::uint16_t mask=controller_mask();
        if(type==oblivion::protocol::MessageType::MotorCommand && action==1 && mask==0){log("Start rejected: select at least one controller.");return;}
        oblivion::protocol::Packet packet{}; packet.type=type; packet.sequence=++sequence_;
        packet.payload={action,static_cast<std::uint8_t>(sprint_->isChecked()?0:1),static_cast<std::uint8_t>(mask&0xffu),static_cast<std::uint8_t>((mask>>8u)&0xffu)};
        const auto bytes=oblivion::protocol::encode(packet);
        const auto written=serial_->isOpen()?serial_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size())):tcp_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size()));
        if(written!=static_cast<qint64>(bytes.size())){log("Transport write failed.");return;}
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
        receive_buffer_.append(serial_->isOpen()?serial_->readAll():tcp_->readAll());
        while(receive_buffer_.size()>=12) {
            const auto* data=reinterpret_cast<const std::uint8_t*>(receive_buffer_.constData());
            if(data[0]!=oblivion::protocol::kMagic0||data[1]!=oblivion::protocol::kMagic1){receive_buffer_.remove(0,1);continue;}
            const int payload=static_cast<int>(data[8])|(static_cast<int>(data[9])<<8); const int frameSize=12+payload;
            if(payload>512){receive_buffer_.remove(0,1);continue;} if(receive_buffer_.size()<frameSize)break;
            oblivion::protocol::Packet packet{};
            if(oblivion::protocol::decode(data,static_cast<std::size_t>(frameSize),packet)){
                ++frames_;bytes_+=static_cast<std::uint64_t>(frameSize);++rate_;
                if(packet.type==oblivion::protocol::MessageType::Telemetry){
                    oblivion::protocol::TelemetryV1 t{};
                    if(oblivion::protocol::decode_telemetry_v1(packet.payload.data(),packet.payload.size(),t)){
                        update_decoded_telemetry(t);
                        log("Typed telemetry v1 decoded.");
                    }else log("Telemetry frame rejected: schema length or field validation failed.");
                }
                QString hex;for(auto b:packet.payload){if(!hex.isEmpty())hex+=" ";hex+=QString("%1").arg(b,2,16,QChar('0')).toUpper();plot_->append(b);}
                int n=rawTable_->rowCount();rawTable_->insertRow(n);
                QStringList vals={QDateTime::currentDateTime().toString("HH:mm:ss.zzz"),QString::number(static_cast<int>(packet.type)),QString::number(packet.sequence),QString::number(packet.payload.size()),hex};
                for(int j=0;j<5;++j)rawTable_->setItem(n,j,new QTableWidgetItem(vals[j]));
                while(rawTable_->rowCount()>500)rawTable_->removeRow(0);
                rxFrames_->setText(QString::number(frames_));rxBytes_->setText(QString::number(bytes_)+" B");
                log("RX frame type "+QString::number(static_cast<int>(packet.type))+" · seq "+QString::number(packet.sequence)+" · "+QString::number(packet.payload.size())+" bytes");
            }else{++errors_;rxErrors_->setText(QString::number(errors_));log("RX invalid frame rejected (CRC/version/length).");}
            receive_buffer_.remove(0,frameSize);
        }
    }
    QComboBox* port_{}; QComboBox* transport_{}; QLineEdit* host_{}; QLineEdit* tcpPort_{}; QLabel* status_{}; QLabel* selectedCount_{}; QTextEdit* telemetry_{};
    QPushButton* refresh_{}; QPushButton* connect_{}; QPushButton* disconnect_{}; QPushButton* sprint_{}; QPushButton* explore_{};
    QPushButton* start_{}; QPushButton* pause_{}; QPushButton* stop_{}; QPushButton* estop_{};
    std::vector<QCheckBox*> controllers_;
    QLabel *rxFrames_{},*rxBytes_{},*rxRate_{},*rxErrors_{};
    SignalPlot* plot_{};QTableWidget *rawTable_{},*decodedTable_{};
    QTimer rateTimer_;std::uint64_t frames_{0},bytes_{0},errors_{0},rate_{0};
    std::unique_ptr<QSerialPort> serial_; std::unique_ptr<QTcpSocket> tcp_; QTimer refresh_timer_; QByteArray receive_buffer_; std::uint32_t sequence_{0};
};

int main(int argc,char** argv) {
    QApplication app(argc,argv);
    QApplication::setApplicationName("OBLIVION Control Center");
    QApplication::setOrganizationName("DMJ");
    OblivionWindow window; window.show(); return app.exec();
}
