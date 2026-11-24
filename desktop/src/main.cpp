#include "oblivion/protocol/frame_codec.hpp"
#include "oblivion/protocol/telemetry.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QByteArray>
#include <QAbstractItemView>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPolygonF>
#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

namespace {
const char* kStyle = R"QSS(
QMainWindow, QWidget#appRoot { background:#0b0918; color:#f4efff; font-family:"Segoe UI"; font-size:13px; }
QWidget#sidebar { background:#100d22; border-right:1px solid #30234d; }
QLabel#brand { color:#fbf7ff; font-size:25px; font-weight:800; letter-spacing:5px; }
QLabel#eyebrow { color:#a99bc8; font-size:10px; font-weight:700; letter-spacing:2px; }
QLabel#muted { color:#a99fbd; font-size:11px; }
QLabel#sectionTitle { color:#f6f0ff; font-size:13px; font-weight:700; }
QLabel#metric { color:#ffffff; font-size:22px; font-weight:700; }
QFrame#surface { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #211a39,stop:0.55 #17142b,stop:1 #111426); border:1px solid #51406e; border-radius:16px; }
QFrame#heroSurface { background:#151027; border:1px solid #7650a9; border-radius:20px; }
QPushButton { background:#211b38; color:#eee7ff; border:1px solid #54436f; border-radius:11px; padding:10px 15px; font-weight:600; }
QPushButton:hover { background:#38245b; border-color:#a16aff; }
QPushButton:disabled { color:#665d79; background:#171326; border-color:#302744; }
QPushButton#primary { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #b65cff,stop:1 #6334c8); color:white; border:1px solid #d6a5ff; padding:12px 22px; }
QPushButton#primary:hover { background:#c17aff; }
QPushButton#danger { background:#401b36; color:#ffb8d1; border:1px solid #b84478; padding:12px 18px; }
QPushButton#profile { background:#1c1730; color:#cfc3e9; border:1px solid #49385f; padding:12px 16px; text-align:left; }
QPushButton#profile:checked { background:#43216c; color:#ffffff; border:1px solid #b76aff; }
QPushButton#nav { background:transparent; color:#c9bfdc; border:1px solid transparent; text-align:left; padding:11px 14px; }
QPushButton#nav:checked { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #6536aa,stop:1 #38245d); color:#ffffff; border:1px solid #9f67e7; }
QPushButton#nav:hover { background:#241b3b; }
QComboBox, QLineEdit { background:#110e20; color:#f3ecff; border:1px solid #594775; border-radius:9px; padding:8px 10px; min-height:18px; }
QComboBox::drop-down { border:0; width:24px; }
QCheckBox { color:#e4daf5; spacing:8px; padding:5px; }
QCheckBox::indicator { width:16px; height:16px; }
QTextEdit, QTableWidget { background:#100e1d; border:1px solid #392d50; border-radius:10px; padding:8px; color:#e8e0f7; font-family:Consolas; font-size:11px; gridline-color:#302744; }
QHeaderView::section { background:#241b3b; color:#cdbde9; border:0; padding:7px; }
QProgressBar { background:#0d0b18; border:1px solid #44345f; border-radius:5px; height:8px; text-align:center; color:transparent; }
QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8c4be5,stop:1 #e06dff); border-radius:4px; }
QTabWidget::pane { border:0; }
QStatusBar { background:#100d20; color:#bcaed7; border-top:1px solid #3a2a54; }
)QSS";

QLabel* label(const QString& text, const char* objectName=nullptr) {
    auto* out = new QLabel(text);
    if (objectName) out->setObjectName(objectName);
    return out;
}
void addHeading(QVBoxLayout* layout, const QString& text, const QString& hint={}) {
    auto* row = new QHBoxLayout;
    row->addWidget(label(text, "sectionTitle"));
    row->addStretch();
    if (!hint.isEmpty()) row->addWidget(label(hint, "muted"));
    layout->addLayout(row);
}
}

class HeroArtwork final : public QWidget {
public:
    explicit HeroArtwork(QWidget* parent=nullptr):QWidget(parent){setMinimumHeight(220);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const QRectF r=rect();
        QLinearGradient bg(r.topLeft(),r.bottomRight()); bg.setColorAt(0,QColor("#211638"));bg.setColorAt(.55,QColor("#17142c"));bg.setColorAt(1,QColor("#0e1426"));p.fillRect(r,bg);
        QRadialGradient glow(QPointF(r.width()*.78,r.height()*.38),r.width()*.43);glow.setColorAt(0,QColor(174,75,255,105));glow.setColorAt(1,QColor(105,54,180,0));p.fillRect(r,glow);
        p.setPen(QPen(QColor(177,137,231,24),1));for(int x=0;x<width();x+=34)p.drawLine(x,0,x,height());for(int y=0;y<height();y+=34)p.drawLine(0,y,width(),y);
        QPainterPath track;track.moveTo(r.width()*.48,r.height()*1.05);track.cubicTo(r.width()*.55,r.height()*.72,r.width()*.37,r.height()*.66,r.width()*.55,r.height()*.42);track.cubicTo(r.width()*.68,r.height()*.22,r.width()*.83,r.height()*.28,r.width()*.95,r.height()*.08);
        p.setPen(QPen(QColor(193,107,255,38),32,Qt::SolidLine,Qt::RoundCap));p.drawPath(track);p.setPen(QPen(QColor("#e5b7ff"),3,Qt::SolidLine,Qt::RoundCap));p.drawPath(track);
        const QPointF c(r.width()*.76,r.height()*.56);
        p.save();p.translate(c);p.rotate(-9);
        p.setPen(Qt::NoPen);p.setBrush(QColor(8,7,18,150));p.drawEllipse(QRectF(-128,65,260,36));
        QRadialGradient under(0,80,150);under.setColorAt(0,QColor(175,65,255,110));under.setColorAt(1,QColor(175,65,255,0));p.setBrush(under);p.drawEllipse(QRectF(-150,42,300,88));
        p.setBrush(QColor("#080b15"));p.setPen(QPen(QColor("#a77be5"),3));p.drawRoundedRect(QRectF(-116,-24,38,108),13,13);p.drawRoundedRect(QRectF(80,-24,38,108),13,13);
        p.setBrush(QColor("#202a3d"));p.setPen(QPen(QColor("#b18adf"),2));p.drawRoundedRect(QRectF(-98,-48,196,104),20,20);
        QPolygonF deck;deck<<QPointF(-84,-45)<<QPointF(-56,-78)<<QPointF(55,-78)<<QPointF(86,-45)<<QPointF(70,28)<<QPointF(-70,28);
        p.setBrush(QColor("#28354b"));p.setPen(QPen(QColor("#c9a4f4"),2));p.drawPolygon(deck);
        p.setBrush(QColor("#0c1422"));p.drawRoundedRect(QRectF(-38,-69,76,38),7,7);
        p.setPen(QPen(QColor("#c16cff"),2));p.drawLine(-22,-60,22,-60);p.drawLine(-22,-52,22,-52);
        p.setBrush(QColor("#d34dff"));p.setPen(Qt::NoPen);for(int i=-2;i<=2;++i)p.drawEllipse(QPointF(i*23,38),5,5);
        p.setBrush(QColor("#f0b83e"));p.drawRoundedRect(QRectF(-60,-87,120,10),4,4);
        p.setBrush(QColor("#090d17"));p.setPen(QPen(QColor("#8b9cb9"),3));p.drawEllipse(QRectF(-107,-31,22,66));p.drawEllipse(QRectF(85,-31,22,66));
        p.restore();
        p.setPen(QColor("#f7efff"));QFont f=p.font();f.setPointSize(19);f.setBold(true);p.setFont(f);p.drawText(QPointF(28,48),"AUTONOMOUS LINE FOLLOWER PLATFORM");
        f.setPointSize(10);f.setBold(false);p.setFont(f);p.setPen(QColor("#c7b8df"));p.drawText(QPointF(30,70),"REAL-TIME CONTROL  ·  PERCEPTION  ·  NAVIGATION  ·  TESTING");
        p.setPen(QPen(QColor("#b26bff"),1));p.setBrush(QColor(27,19,47,225));p.drawRoundedRect(QRectF(26,height()-47,245,28),14,14);p.setPen(QColor("#d6b7ff"));p.drawText(QRectF(38,height()-45,225,24),Qt::AlignVCenter,"●  VIRTUAL ROBOT READY");
    }
};

class SignalPlot final : public QWidget {
public:
 explicit SignalPlot(QWidget* p=nullptr):QWidget(p){setMinimumHeight(140);}
 void append(double v){samples_.push_back(v);while(samples_.size()>240)samples_.pop_front();update();}
protected:
 void paintEvent(QPaintEvent*) override {
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#100e1d"));QRectF a=rect().adjusted(42,20,-12,-20);
  p.setPen(QColor("#9b8bb5"));p.drawText(12,15,"SIGNAL HISTORY");
  p.setPen(QColor("#302744"));for(int i=0;i<=4;++i){qreal y=a.top()+a.height()*i/4.0;p.drawLine(QPointF(a.left(),y),QPointF(a.right(),y));}
  if(samples_.size()<2){p.setPen(QColor("#9b8bb5"));p.drawText(a,Qt::AlignCenter,"Waiting for incoming telemetry");return;}
  auto mm=std::minmax_element(samples_.begin(),samples_.end());double lo=*mm.first,hi=*mm.second;if(hi-lo<1){lo-=1;hi+=1;}
  QPolygonF line;for(std::size_t i=0;i<samples_.size();++i){qreal x=a.left()+a.width()*i/(samples_.size()-1);qreal y=a.bottom()-(samples_[i]-lo)/(hi-lo)*a.height();line<<QPointF(x,y);}
  p.setPen(QPen(QColor("#cf70ff"),2));p.drawPolyline(line);
 }
private:std::deque<double> samples_;
};

class TrackPreview final : public QWidget {
public:
 explicit TrackPreview(QWidget* p=nullptr):QWidget(p){setMinimumHeight(110);}
protected:
 void paintEvent(QPaintEvent*) override {
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#100e1e"));
  p.setPen(QPen(QColor("#2f2944"),1));for(int x=0;x<width();x+=24)p.drawLine(x,0,x,height());for(int y=0;y<height();y+=24)p.drawLine(0,y,width(),y);
  QPainterPath path;path.moveTo(18,height()*.72);path.cubicTo(width()*.18,height()*.12,width()*.27,height()*.88,width()*.42,height()*.52);path.cubicTo(width()*.56,height()*.14,width()*.62,height()*.22,width()*.7,height()*.5);path.cubicTo(width()*.8,height()*.84,width()*.87,height()*.22,width()-18,height()*.28);
  p.setPen(QPen(QColor(187,93,255,55),12,Qt::SolidLine,Qt::RoundCap));p.drawPath(path);p.setPen(QPen(QColor("#e6caff"),3,Qt::SolidLine,Qt::RoundCap));p.drawPath(path);
  p.setBrush(QColor("#c77aff"));p.setPen(QPen(Qt::white,1));p.drawEllipse(QPointF(18,height()*.72),5,5);
 }
};

class OblivionWindow final : public QMainWindow {
public:
 OblivionWindow(){
  setWindowTitle("OBLIVION Control Center");resize(1640,980);setMinimumSize(1200,760);setStyleSheet(kStyle);
  serial_=std::make_unique<QSerialPort>();tcp_=std::make_unique<QTcpSocket>();
  tabs_=new QTabWidget;tabs_->tabBar()->hide();
  auto* shell=new QWidget; shell->setObjectName("appRoot");auto* shellRow=new QHBoxLayout(shell);shellRow->setContentsMargins(0,0,0,0);shellRow->setSpacing(0);
  auto* sidebar=new QFrame;sidebar->setObjectName("sidebar");sidebar->setFixedWidth(248);auto* side=new QVBoxLayout(sidebar);side->setContentsMargins(17,24,17,20);side->setSpacing(9);
  auto* mark=label("DMJ   /   AUTONOMOUS SYSTEMS","eyebrow");side->addWidget(mark);side->addSpacing(18);
  auto* logo=label("◢  OBLIVION","brand");side->addWidget(logo);side->addWidget(label("Robotics Control Center","muted"));side->addSpacing(24);
  auto* navTitle=label("WORKSPACE","eyebrow");side->addWidget(navTitle);
  auto* navHome=new QPushButton("⌂    Control Center");navHome->setObjectName("nav");navHome->setCheckable(true);navHome->setChecked(true);side->addWidget(navHome);
  auto* navMonitor=new QPushButton("◉    Live Monitor");navMonitor->setObjectName("nav");navMonitor->setCheckable(true);side->addWidget(navMonitor);
  auto* navLine=new QPushButton("⌁    Line Follower");navLine->setObjectName("nav");navLine->setEnabled(false);side->addWidget(navLine);
  auto* navCal=new QPushButton("◎    Calibration");navCal->setObjectName("nav");navCal->setEnabled(false);side->addWidget(navCal);
  auto* navTune=new QPushButton("☷    PID Tuning");navTune->setObjectName("nav");navTune->setEnabled(false);side->addWidget(navTune);
  auto* navSim=new QPushButton("◇    Simulation");navSim->setObjectName("nav");side->addWidget(navSim);
  auto* navLogs=new QPushButton("▥    Logs & Telemetry");navLogs->setObjectName("nav");navLogs->setEnabled(false);side->addWidget(navLogs);
  auto* navSettings=new QPushButton("⚙    Settings");navSettings->setObjectName("nav");navSettings->setEnabled(false);side->addWidget(navSettings);
  side->addStretch();side->addWidget(label("DMJ AUTONOMOUS SYSTEMS","eyebrow"));side->addWidget(label("BEYOND REACTION. BEYOND LIMITS.","muted"));
  shellRow->addWidget(sidebar);
  auto* main=new QWidget;auto* mainCol=new QVBoxLayout(main);mainCol->setContentsMargins(24,18,24,14);mainCol->setSpacing(13);
  auto* top=new QHBoxLayout;auto* titleCol=new QVBoxLayout;titleCol->setSpacing(3);titleCol->addWidget(label("O B L I V I O N","brand"));titleCol->addWidget(label("Autonomous Line Follower Platform   ·   Real-Time Control  ·  Perception  ·  Navigation  ·  Testing","muted"));top->addLayout(titleCol);top->addStretch();
  auto* clock=label(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy   |   HH:mm"),"muted");top->addWidget(clock);mainCol->addLayout(top);
  auto* scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);auto* content=new QWidget;auto* body=new QVBoxLayout(content);body->setContentsMargins(2,4,2,12);body->setSpacing(13);
  auto* hero=new QFrame;hero->setObjectName("heroSurface");auto* heroLay=new QVBoxLayout(hero);heroLay->setContentsMargins(0,0,0,0);heroArt_=new HeroArtwork;heroLay->addWidget(heroArt_);body->addWidget(hero);
  auto* connectCard=surface();auto* connectLay=new QVBoxLayout(connectCard);connectLay->setContentsMargins(14,10,14,12);addHeading(connectLay,"ROBOT CONNECTION","SELECT TRANSPORT");auto* conn=new QHBoxLayout;
  transport_=new QComboBox;transport_->addItems({"Physical robot  ·  Serial","Standalone simulator  ·  TCP"});
  port_=new QComboBox;port_->setMinimumWidth(150);host_=new QLineEdit("127.0.0.1");host_->setMaximumWidth(145);tcpPort_=new QLineEdit("8765");tcpPort_->setMaximumWidth(78);
  refresh_=new QPushButton("Refresh");connect_=new QPushButton("Connect");connect_->setObjectName("primary");disconnect_=new QPushButton("Disconnect");disconnect_->setEnabled(false);
  status_=label("●  OFFLINE","statusDisconnected");conn->addWidget(transport_);conn->addWidget(port_,1);conn->addWidget(host_);conn->addWidget(tcpPort_);conn->addWidget(refresh_);conn->addWidget(connect_);conn->addWidget(disconnect_);conn->addWidget(status_);connectLay->addLayout(conn);body->addWidget(connectCard);
  auto* profileCard=surface();auto* profileLay=new QVBoxLayout(profileCard);profileLay->setContentsMargins(14,10,14,12);addHeading(profileLay,"MISSION PROFILE / CONTROL MODE");
  auto* profiles=new QHBoxLayout;sprint_=new QPushButton("SPRINT  ·  High-speed line following");explore_=new QPushButton("EXPLORE  ·  Maze mapping & route solving");sprint_->setObjectName("profile");explore_->setObjectName("profile");sprint_->setCheckable(true);explore_->setCheckable(true);sprint_->setChecked(true);profiles->addWidget(sprint_,1);profiles->addWidget(explore_,1);
  auto* modeBox=new QHBoxLayout;autoLabel_=new QPushButton("◉  Automatic");autoLabel_->setObjectName("profile");autoLabel_->setCheckable(true);autoLabel_->setChecked(true);manualLabel_=new QPushButton("⌘  Manual");manualLabel_->setObjectName("profile");manualLabel_->setEnabled(false);simMode_=new QPushButton("◇  Simulation");simMode_->setObjectName("profile");simMode_->setCheckable(true);modeBox->addWidget(autoLabel_);modeBox->addWidget(manualLabel_);modeBox->addWidget(simMode_);profiles->addLayout(modeBox);profileLay->addLayout(profiles);body->addWidget(profileCard);
  auto* actions=new QHBoxLayout;start_=new QPushButton("▶   Start");start_->setObjectName("primary");pause_=new QPushButton("Ⅱ   Pause");stop_=new QPushButton("□   Stop");estop_=new QPushButton("⚠   Emergency Stop");estop_->setObjectName("danger");calibrate_=new QPushButton("◎  Calibrate");heartbeat_=new QPushButton("Heartbeat");start_->setEnabled(false);pause_->setEnabled(false);stop_->setEnabled(false);estop_->setEnabled(false);actions->addWidget(start_);actions->addWidget(pause_);actions->addWidget(stop_);actions->addWidget(estop_);actions->addStretch();actions->addWidget(calibrate_);actions->addWidget(heartbeat_);body->addLayout(actions);
  auto* grid=new QGridLayout;grid->setHorizontalSpacing(12);grid->setVerticalSpacing(12);
  auto* liveCard=surface();auto* liveLay=new QVBoxLayout(liveCard);liveLay->setContentsMargins(12,10,12,12);addHeading(liveLay,"LIVE VIEW  /  TRACK PREVIEW","SIMULATED VIEW UNTIL CAMERA CONNECTED");trackPreview_=new TrackPreview;liveLay->addWidget(trackPreview_);grid->addWidget(liveCard,0,0);
  auto* lineCard=surface();auto* lineLay=new QVBoxLayout(lineCard);lineLay->setContentsMargins(12,10,12,12);addHeading(lineLay,"LINE DETECTION","REFLECTANCE ARRAY");linePosition_=label("— mm","metric");lineConfidence_=label("— %","metric");auto* lineStats=new QHBoxLayout;lineStats->addWidget(label("Offset","muted"));lineStats->addWidget(linePosition_);lineStats->addSpacing(12);lineStats->addWidget(label("Confidence","muted"));lineStats->addWidget(lineConfidence_);lineLay->addLayout(lineStats);auto* bars=new QHBoxLayout;for(int i=0;i<8;++i){auto* b=new QProgressBar;b->setRange(0,1000);b->setValue(0);b->setTextVisible(false);b->setFixedHeight(11);sensorBars_.push_back(b);bars->addWidget(b);}lineLay->addWidget(bars);grid->addWidget(lineCard,0,1);
  auto* robotCard=surface();auto* robotLay=new QVBoxLayout(robotCard);robotLay->setContentsMargins(12,10,12,12);addHeading(robotLay,"ROBOT STATUS","LIVE TELEMETRY");auto* statusGrid=new QGridLayout;speedValue_=metricCard(statusGrid,0,0,"SPEED","— m/s");batteryValue_=metricCard(statusGrid,0,1,"BATTERY","— V");cpuValue_=metricCard(statusGrid,1,0,"CPU LOAD","— %");faultValue_=metricCard(statusGrid,1,1,"FAULT FLAGS","—");robotLay->addLayout(statusGrid);grid->addWidget(robotCard,0,2);
  grid->setColumnStretch(0,1);grid->setColumnStretch(1,1);grid->setColumnStretch(2,1);body->addLayout(grid);
  auto* lower=new QGridLayout;auto* controllerCard=surface();auto* controllerLay=new QVBoxLayout(controllerCard);controllerLay->setContentsMargins(12,10,12,12);addHeading(controllerLay,"PARALLEL CONTROLLER ENSEMBLE","SUPERVISED CONTROL");
  const std::array<const char*,11> names={"PID","LQR","MPC","Sliding Mode","H-infinity","ADRC","Backstepping","Adaptive PD","Gain Scheduled","Fuzzy","Robust Feedback"};
  const std::array<const char*,11> desc={"Baseline tracking","Optimal feedback","Predictive control","Robust nonlinear","Robust synthesis","Disturbance observer","Nonlinear control","Online adaptation","Operating schedule","Rule-based control","Fallback controller"};
  auto* cg=new QGridLayout;for(std::size_t i=0;i<names.size();++i){auto* item=new QWidget;auto* il=new QVBoxLayout(item);il->setContentsMargins(2,1,2,1);auto* check=new QCheckBox(names[i]);auto* d=label(desc[i],"muted");il->addWidget(check);il->addWidget(d);cg->addWidget(item,static_cast<int>(i/3),static_cast<int>(i%3));controllers_.push_back(check);connect(check,&QCheckBox::toggled,this,[this]{update_controller_count();});}controllerLay->addLayout(cg);
  auto* sel=new QHBoxLayout;auto* recommended=new QPushButton("Select recommended");auto* clear=new QPushButton("Clear");selectedCount_=label("0 SELECTED","eyebrow");sel->addWidget(recommended);sel->addWidget(clear);sel->addStretch();sel->addWidget(selectedCount_);controllerLay->addLayout(sel);lower->addWidget(controllerCard,0,0,2,1);
  auto* plotCard=surface();auto* plotLay=new QVBoxLayout(plotCard);plotLay->setContentsMargins(12,10,12,12);addHeading(plotLay,"TELEMETRY  /  REAL TIME","LIVE SIGNALS");plot_=new SignalPlot;plotLay->addWidget(plot_);lower->addWidget(plotCard,0,1);
  auto* trackCard=surface();auto* trackLay=new QVBoxLayout(trackCard);trackLay->setContentsMargins(12,10,12,12);addHeading(trackLay,"TRACK & MISSION","CURRENT PROFILE");trackLay->addWidget(new TrackPreview);missionInfo_=label("Track: Awaiting telemetry\nMode: Sprint\nLaps: —\nDistance: —\nStatus: Disconnected","muted");trackLay->addWidget(missionInfo_);lower->addWidget(trackCard,1,1);
  auto* safetyCard=surface();auto* safetyLay=new QVBoxLayout(safetyCard);safetyLay->setContentsMargins(12,10,12,12);addHeading(safetyLay,"SAFETY INTERLOCK");safetyLay->addWidget(label("Emergency stop is a request to firmware. The target must independently enforce watchdog timeouts, motor-off startup defaults and a hardware-safe E-stop.","muted"));lower->addWidget(safetyCard,1,1);
  lower->setColumnStretch(0,3);lower->setColumnStretch(1,2);body->addLayout(lower);
  auto* eventsCard=surface();auto* eventsLay=new QVBoxLayout(eventsCard);eventsLay->setContentsMargins(12,10,12,12);addHeading(eventsLay,"EVENT & TELEMETRY CONSOLE");telemetry_=new QTextEdit;telemetry_->setReadOnly(true);telemetry_->setMaximumHeight(110);telemetry_->setPlaceholderText("Waiting for a robot or simulator connection…");eventsLay->addWidget(telemetry_);body->addWidget(eventsCard);
  scroll->setWidget(content);mainCol->addWidget(scroll,1);
  auto* footer=new QHBoxLayout;footer->addWidget(label("●  READY","eyebrow"));footer->addWidget(label("  |  OBLIVION v0.2.0-alpha","muted"));footer->addStretch();footer->addWidget(label("DMJ AUTONOMOUS SYSTEMS","eyebrow"));mainCol->addLayout(footer);
  shellRow->addWidget(main,1);tabs_->addTab(shell,"Control Center");
  auto* monitor=new QWidget;auto* ml=new QVBoxLayout(monitor);ml->setContentsMargins(24,20,24,18);ml->setSpacing(12);ml->addWidget(label("LIVE SYSTEM MONITOR","brand"));
  auto* metrics=new QGridLayout;rxFrames_=metricCard(metrics,0,0,"RECEIVED FRAMES","0");rxBytes_=metricCard(metrics,0,1,"RECEIVED BYTES","0 B");rxRate_=metricCard(metrics,0,2,"FRAME RATE","0 /s");rxErrors_=metricCard(metrics,0,3,"INVALID FRAMES","0");ml->addLayout(metrics);
  auto* monitorPlot=surface();auto* mpl=new QVBoxLayout(monitorPlot);mpl->setContentsMargins(12,10,12,12);addHeading(mpl,"RAW SIGNAL TRACE");mpl->addWidget(plot_);ml->addWidget(monitorPlot,1);
  auto* tables=new QHBoxLayout;auto* rawCard=surface();auto* rawLay=new QVBoxLayout(rawCard);addHeading(rawLay,"RAW PACKET INSPECTOR");rawTable_=new QTableWidget(0,5);rawTable_->setHorizontalHeaderLabels({"Time","Type","Sequence","Length","Payload HEX"});rawTable_->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);rawTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);rawTable_->verticalHeader()->hide();rawLay->addWidget(rawTable_);tables->addWidget(rawCard,3);
  auto* decodedCard=surface();auto* decodedLay=new QVBoxLayout(decodedCard);addHeading(decodedLay,"DECODED / DERIVED PARAMETERS");decodedTable_=new QTableWidget(0,3);decodedTable_->setHorizontalHeaderLabels({"Parameter","Value","Source"});decodedTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);decodedTable_->verticalHeader()->hide();decodedTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  const QStringList rows={"Line position|—|Awaiting telemetry","Line confidence|—|Awaiting telemetry","Encoder counts|—|Awaiting telemetry","Linear velocity|—|Awaiting telemetry","Yaw rate|—|Awaiting telemetry","Acceleration|—|Awaiting telemetry","Curvature|—|Awaiting telemetry","Controller outputs|—|Awaiting telemetry","Execution time|—|Awaiting telemetry","CPU / RAM|—|Awaiting telemetry","Deadline misses|—|Awaiting telemetry","Battery voltage|—|Awaiting telemetry"};for(const auto& row:rows){auto f=row.split("|");int n=decodedTable_->rowCount();decodedTable_->insertRow(n);for(int j=0;j<3;++j)decodedTable_->setItem(n,j,new QTableWidgetItem(f[j]));}decodedLay->addWidget(decodedTable_);tables->addWidget(decodedCard,2);ml->addLayout(tables,2);tabs_->addTab(monitor,"Live Monitor");setCentralWidget(tabs_);
  connect(navHome,&QPushButton::clicked,this,[this,navHome,navMonitor]{tabs_->setCurrentIndex(0);navHome->setChecked(true);navMonitor->setChecked(false);});
  connect(navMonitor,&QPushButton::clicked,this,[this,navHome,navMonitor]{tabs_->setCurrentIndex(1);navHome->setChecked(false);navMonitor->setChecked(true);});
  connect(navSim,&QPushButton::clicked,this,[this]{transport_->setCurrentIndex(1);host_->setFocus();statusBar()->showMessage("Simulator transport selected · start the standalone simulator process first.");});
  connect(transport_,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int m){const bool tcp=m==1;port_->setVisible(!tcp);refresh_->setVisible(!tcp);host_->setVisible(tcp);tcpPort_->setVisible(tcp);simMode_->setChecked(tcp);});
  host_->hide();tcpPort_->hide();refresh_ports();
  connect(refresh_,&QPushButton::clicked,this,[this]{refresh_ports();});
  connect(connect_,&QPushButton::clicked,this,[this]{open_port();});
  connect(disconnect_,&QPushButton::clicked,this,[this]{close_port();});
  connect(sprint_,&QPushButton::clicked,this,[this]{sprint_->setChecked(true);explore_->setChecked(false);update_mission_label();});
  connect(explore_,&QPushButton::clicked,this,[this]{explore_->setChecked(true);sprint_->setChecked(false);update_mission_label();});
  connect(autoLabel_,&QPushButton::clicked,this,[this]{autoLabel_->setChecked(true);send_log("Automatic controller mode selected.");});
  connect(simMode_,&QPushButton::clicked,this,[this]{transport_->setCurrentIndex(1);send_log("Simulation transport selected. Connect to the separate simulator process.");});
  connect(recommended,&QPushButton::clicked,this,[this]{for(std::size_t i=0;i<controllers_.size();++i)controllers_[i]->setChecked(i==0||i==1||i==2||i==4||i==5||i==9);});
  connect(clear,&QPushButton::clicked,this,[this]{for(auto* c:controllers_)c->setChecked(false);});
  connect(start_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,1);});
  connect(pause_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,2);});
  connect(stop_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,0);});
  connect(estop_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::EmergencyStop,0);});
  connect(calibrate_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::Calibration,0x10);});
  connect(heartbeat_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::Heartbeat,0);});
  connect(tcp_.get(),&QTcpSocket::readyRead,this,[this]{read_serial();});
  connect(serial_.get(),&QSerialPort::readyRead,this,[this]{read_serial();});
  connect(serial_.get(),&QSerialPort::errorOccurred,this,[this](QSerialPort::SerialPortError e){if(e!=QSerialPort::NoError)send_log("Serial error: "+serial_->errorString());});
  refreshTimer_.setInterval(2000);connect(&refreshTimer_,&QTimer::timeout,this,[this]{if(!serial_->isOpen()&&transport_->currentIndex()==0)refresh_ports();});refreshTimer_.start();
  rateTimer_.setInterval(1000);connect(&rateTimer_,&QTimer::timeout,this,[this]{rxRate_->setText(QString::number(rate_)+"/s");rate_=0;});rateTimer_.start();
  update_controller_count();update_mission_label();statusBar()->showMessage("Ready · No robot connected");
 }
 ~OblivionWindow() override {if(serial_&&serial_->isOpen())serial_->close();if(tcp_&&tcp_->state()!=QAbstractSocket::UnconnectedState)tcp_->abort();}
private:
 QFrame* surface(){auto* f=new QFrame;f->setObjectName("surface");return f;}
 QLabel* metricCard(QGridLayout* grid,int row,int col,const QString& title,const QString& value){auto* card=surface();auto* l=new QVBoxLayout(card);l->setContentsMargins(12,9,12,9);l->addWidget(label(title,"eyebrow"));auto* v=label(value,"metric");l->addWidget(v);grid->addWidget(card,row,col);return v;}
 void send_log(const QString& s){telemetry_->append("["+QDateTime::currentDateTime().toString("HH:mm:ss.zzz")+"]  "+s);}
 void update_mission_label(){if(missionInfo_)missionInfo_->setText(QString("Track: Awaiting telemetry\nMode: %1\nLaps: —\nDistance: —\nStatus: %2").arg(sprint_->isChecked()?"Sprint":"Explore").arg(connected()?"Connected":"Disconnected"));}
 void refresh_ports(){const QString previous=port_->currentData().toString();port_->clear();for(const auto& p:QSerialPortInfo::availablePorts())port_->addItem(p.portName()+"  ·  "+p.description(),p.portName());const int i=port_->findData(previous);if(i>=0)port_->setCurrentIndex(i);}
 bool connected()const{return serial_->isOpen()||(tcp_&&tcp_->state()==QAbstractSocket::ConnectedState);}
 void update_controller_count(){int n=0;for(auto* c:controllers_)if(c->isChecked())++n;selectedCount_->setText(QString::number(n)+" SELECTED");start_->setEnabled(connected()&&n>0);}
 std::uint16_t controller_mask()const{std::uint16_t m=0;for(std::size_t i=0;i<controllers_.size();++i)if(controllers_[i]->isChecked())m|=static_cast<std::uint16_t>(1u<<i);return m;}
 void open_port(){
  if(transport_->currentIndex()==1){bool ok=false;int p=tcpPort_->text().toInt(&ok);if(!ok||p<1||p>65535){QMessageBox::warning(this,"Invalid endpoint","Enter a valid TCP port.");return;}tcp_->connectToHost(host_->text().trimmed(),static_cast<quint16>(p));if(!tcp_->waitForConnected(2500)){QMessageBox::critical(this,"Simulator connection failed",tcp_->errorString());return;}send_log("Simulator TCP connected · "+host_->text()+":"+QString::number(p));}
  else{if(port_->currentData().toString().isEmpty()){QMessageBox::warning(this,"No serial interface","Connect the robot controller and select its serial interface.");return;}serial_->setPortName(port_->currentData().toString());serial_->setBaudRate(QSerialPort::Baud115200);serial_->setDataBits(QSerialPort::Data8);serial_->setParity(QSerialPort::NoParity);serial_->setStopBits(QSerialPort::OneStop);serial_->setFlowControl(QSerialPort::NoFlowControl);if(!serial_->open(QIODevice::ReadWrite)){QMessageBox::critical(this,"Connection failed",serial_->errorString());return;}send_log("Physical serial link opened · "+serial_->portName());}
  status_->setText(transport_->currentIndex()==1?"●  SIMULATOR ONLINE":"●  ROBOT CONNECTED");status_->setObjectName("statusConnected");status_->style()->unpolish(status_);status_->style()->polish(status_);connect_->setEnabled(false);disconnect_->setEnabled(true);transport_->setEnabled(false);pause_->setEnabled(true);stop_->setEnabled(true);estop_->setEnabled(true);update_controller_count();update_mission_label();statusBar()->showMessage("Connected · protocol v1 · telemetry stream active");send_action(oblivion::protocol::MessageType::Heartbeat,0);
 }
 void close_port(){if(serial_->isOpen())serial_->close();if(tcp_&&tcp_->state()!=QAbstractSocket::UnconnectedState)tcp_->disconnectFromHost();status_->setText("●  OFFLINE");status_->setObjectName("statusDisconnected");status_->style()->unpolish(status_);status_->style()->polish(status_);connect_->setEnabled(true);disconnect_->setEnabled(false);transport_->setEnabled(true);pause_->setEnabled(false);stop_->setEnabled(false);estop_->setEnabled(false);update_controller_count();update_mission_label();statusBar()->showMessage("Disconnected · target firmware watchdog remains responsible for motor safety");send_log("Transport disconnected.");}
 void send_action(oblivion::protocol::MessageType type,std::uint8_t action){if(!connected()){send_log("Rejected: no robot connection.");return;}const auto mask=controller_mask();if(type==oblivion::protocol::MessageType::MotorCommand&&action==1&&mask==0){send_log("Start rejected: select at least one controller.");return;}oblivion::protocol::Packet packet{};packet.type=type;packet.sequence=++sequence_;packet.payload={action,static_cast<std::uint8_t>(sprint_->isChecked()?0:1),static_cast<std::uint8_t>(mask&0xffu),static_cast<std::uint8_t>((mask>>8u)&0xffu)};const auto bytes=oblivion::protocol::encode(packet);const auto written=serial_->isOpen()?serial_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size())):tcp_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size()));if(written!=static_cast<qint64>(bytes.size())){send_log("Transport write failed.");return;}const QString name=type==oblivion::protocol::MessageType::EmergencyStop?"EMERGENCY STOP":type==oblivion::protocol::MessageType::Heartbeat?"HEARTBEAT":type==oblivion::protocol::MessageType::Calibration?"CALIBRATION":action==1?"START REQUEST":action==2?"PAUSE REQUEST":"STOP REQUEST";send_log("TX "+name+" · seq "+QString::number(sequence_)+" · profile "+(sprint_->isChecked()?"Sprint":"Explore")+" · controller mask 0x"+QString::number(mask,16).toUpper());}
 void set_parameter(int row,const QString& value,const QString& source){decodedTable_->item(row,1)->setText(value);decodedTable_->item(row,2)->setText(source);}
 void update_decoded_telemetry(const oblivion::protocol::TelemetryV1& t){
  using namespace oblivion::protocol;const auto has=[&](TelemetryValidity bit){return(t.validity&static_cast<std::uint32_t>(bit))!=0;};
  if(has(ValidLineEstimate)){const QString pos=QString::number(t.line_position_um/1000.0,'f',2);const QString conf=QString::number(t.line_confidence_permille/10.0,'f',1);linePosition_->setText(pos+" mm");lineConfidence_->setText(conf+" %");set_parameter(0,pos+" mm","Telemetry v1 · line estimate");set_parameter(1,conf+" %","Telemetry v1 · confidence");}
  if(has(ValidReflectance))for(std::size_t i=0;i<sensorBars_.size();++i)sensorBars_[i]->setValue(std::min<int>(1000,t.reflectance_raw[i]));
  if(has(ValidEncoders))set_parameter(2,QString::number(t.left_encoder_ticks)+" / "+QString::number(t.right_encoder_ticks),"Telemetry v1 · encoder ticks");
  if(has(ValidMotion)){const auto speed=t.linear_velocity_mmps/1000.0;speedValue_->setText(QString::number(speed,'f',2)+" m/s");set_parameter(3,QString::number(speed,'f',3)+" m/s","Telemetry v1 · reported velocity");set_parameter(4,QString::number(t.yaw_rate_mradps/1000.0,'f',3)+" rad/s","Telemetry v1 · reported yaw rate");set_parameter(6,QString::number(t.curvature_milli_inv_m/1000.0,'f',4)+" 1/m","Telemetry v1 · reported curvature");}
  if(has(ValidImu)){const QString a=QString("%1, %2, %3 m/s²").arg(t.accel_x_mg*9.80665/1000.0,0,'f',2).arg(t.accel_y_mg*9.80665/1000.0,0,'f',2).arg(t.accel_z_mg*9.80665/1000.0,0,'f',2);set_parameter(5,a,"Telemetry v1 · IMU acceleration");}
  if(has(ValidControllerStats)){QStringList outputs;for(std::size_t i=0;i<t.controllers.size();++i)if((t.active_controller_mask>>i)&1u)outputs<<QString::number(i)+":"+QString::number(t.controllers[i].output_milli/1000.0,'f',2);set_parameter(7,outputs.join("  "),"Telemetry v1 · controller bank");set_parameter(8,QString("max %1 µs").arg([&]{std::uint16_t m=0;for(const auto& c:t.controllers)m=std::max(m,c.execution_time_us);return m;}()),"Telemetry v1 · controller execution");}
  if(has(ValidSystemHealth)){cpuValue_->setText(QString::number(t.cpu_load_percent)+" %");set_parameter(9,QString("%1 % / %2 B").arg(t.cpu_load_percent).arg(t.free_memory_bytes),"Telemetry v1 · target system");set_parameter(10,t.fault_flags?"Fault flags 0x"+QString::number(t.fault_flags,16):"0","Telemetry v1 · fault flags");faultValue_->setText(t.fault_flags?"0x"+QString::number(t.fault_flags,16):"NONE");}
  if(has(ValidPower)){batteryValue_->setText(QString::number(t.battery_millivolts/1000.0,'f',2)+" V");set_parameter(11,QString::number(t.battery_millivolts/1000.0,'f',2)+" V","Telemetry v1 · target voltage");}
  if(has(ValidLineEstimate))plot_->append(t.line_position_um/1000.0);
 }
 void read_serial(){receive_buffer_.append(serial_->isOpen()?serial_->readAll():tcp_->readAll());while(receive_buffer_.size()>=12){const auto* data=reinterpret_cast<const std::uint8_t*>(receive_buffer_.constData());if(data[0]!=oblivion::protocol::kMagic0||data[1]!=oblivion::protocol::kMagic1){receive_buffer_.remove(0,1);continue;}const int payload=static_cast<int>(data[8])|(static_cast<int>(data[9])<<8);const int size=12+payload;if(payload>512){receive_buffer_.remove(0,1);continue;}if(receive_buffer_.size()<size)break;oblivion::protocol::Packet packet{};if(oblivion::protocol::decode(data,static_cast<std::size_t>(size),packet)){++frames_;bytes_+=static_cast<std::uint64_t>(size);++rate_;if(packet.type==oblivion::protocol::MessageType::Telemetry){oblivion::protocol::TelemetryV1 t{};if(oblivion::protocol::decode_telemetry_v1(packet.payload.data(),packet.payload.size(),t))update_decoded_telemetry(t);else send_log("Telemetry frame rejected: invalid schema.");}QString hex;for(auto b:packet.payload){if(!hex.isEmpty())hex+=" ";hex+=QString("%1").arg(b,2,16,QChar('0')).toUpper();}const int n=rawTable_->rowCount();rawTable_->insertRow(n);const QStringList vals={QDateTime::currentDateTime().toString("HH:mm:ss.zzz"),QString::number(static_cast<int>(packet.type)),QString::number(packet.sequence),QString::number(packet.payload.size()),hex};for(int j=0;j<5;++j)rawTable_->setItem(n,j,new QTableWidgetItem(vals[j]));while(rawTable_->rowCount()>500)rawTable_->removeRow(0);rxFrames_->setText(QString::number(frames_));rxBytes_->setText(QString::number(bytes_)+" B");send_log("RX frame type "+QString::number(static_cast<int>(packet.type))+" · seq "+QString::number(packet.sequence));}else{++errors_;rxErrors_->setText(QString::number(errors_));send_log("Invalid frame rejected (CRC/version/length).");}receive_buffer_.remove(0,size);}}
 QTabWidget* tabs_{};QWidget* heroArt_{};QComboBox* transport_{};QComboBox* port_{};QLineEdit* host_{};QLineEdit* tcpPort_{};QLabel* status_{};QLabel* selectedCount_{};QLabel* linePosition_{};QLabel* lineConfidence_{};QLabel* speedValue_{};QLabel* batteryValue_{};QLabel* cpuValue_{};QLabel* faultValue_{};QLabel* missionInfo_{};QTextEdit* telemetry_{};QPushButton* connect_{};QPushButton* disconnect_{};QPushButton* refresh_{};QPushButton* sprint_{};QPushButton* explore_{};QPushButton* start_{};QPushButton* pause_{};QPushButton* stop_{};QPushButton* estop_{};QPushButton* calibrate_{};QPushButton* heartbeat_{};QPushButton* autoLabel_{};QPushButton* manualLabel_{};QPushButton* simMode_{};
 std::vector<QCheckBox*> controllers_;std::vector<QProgressBar*> sensorBars_;SignalPlot* plot_{};TrackPreview* trackPreview_{};QTableWidget* rawTable_{};QTableWidget* decodedTable_{};QLabel *rxFrames_{},*rxBytes_{},*rxRate_{},*rxErrors_{};
 QTimer refreshTimer_,rateTimer_;std::uint64_t frames_{0},bytes_{0},errors_{0},rate_{0};std::unique_ptr<QSerialPort> serial_;std::unique_ptr<QTcpSocket> tcp_;QByteArray receive_buffer_;std::uint32_t sequence_{0};
};
int main(int argc,char** argv){QApplication app(argc,argv);QApplication::setApplicationName("OBLIVION Control Center");QApplication::setOrganizationName("DMJ");OblivionWindow w;w.show();return app.exec();}
