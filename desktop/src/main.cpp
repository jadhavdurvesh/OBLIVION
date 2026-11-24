#include "oblivion/protocol/frame_codec.hpp"
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QByteArray>
#include <cstdint>
#include <memory>

class OblivionWindow final:public QMainWindow{
public:
 OblivionWindow(){
  setWindowTitle("OBLIVION Control Center");resize(1180,760);
  auto* root=new QWidget;auto* outer=new QVBoxLayout(root);
  auto* header=new QHBoxLayout;auto* brand=new QLabel("<h1>OBLIVION</h1><span>Autonomous Robotics Control Center</span>");
  header->addWidget(brand);header->addStretch();status_=new QLabel("DISCONNECTED");status_->setStyleSheet("font-weight:700;color:#d69e36;padding:8px");header->addWidget(status_);outer->addLayout(header);
  auto* connection=new QGroupBox("Robot connection");auto* conn=new QHBoxLayout(connection);
  port_=new QComboBox;refresh_=new QPushButton("Refresh ports");connect_=new QPushButton("Connect");disconnect_=new QPushButton("Disconnect");disconnect_->setEnabled(false);
  conn->addWidget(new QLabel("Serial port"));conn->addWidget(port_,1);conn->addWidget(refresh_);conn->addWidget(connect_);conn->addWidget(disconnect_);outer->addWidget(connection);
  auto* controls=new QGroupBox("Mission and control");auto* ctl=new QHBoxLayout(controls);
  mode_=new QComboBox;mode_->addItem("Sprint");mode_->addItem("Explore");
  controller_=new QComboBox;controller_->addItems({"PID","LQR","MPC","Sliding Mode","H-infinity","ADRC","Backstepping","Adaptive PD","Gain Scheduled","Fuzzy","Robust Feedback"});
  start_=new QPushButton("Start");pause_=new QPushButton("Pause");stop_=new QPushButton("Stop");estop_=new QPushButton("EMERGENCY STOP");
  pause_->setEnabled(false);stop_->setEnabled(false);start_->setEnabled(false);estop_->setEnabled(false);
  ctl->addWidget(new QLabel("Profile"));ctl->addWidget(mode_);ctl->addWidget(new QLabel("Controller"));ctl->addWidget(controller_,1);ctl->addWidget(start_);ctl->addWidget(pause_);ctl->addWidget(stop_);ctl->addWidget(estop_);outer->addWidget(controls);
  auto* grid=new QHBoxLayout;
  auto* telemetryBox=new QGroupBox("Live telemetry");auto* telemetryLayout=new QVBoxLayout(telemetryBox);
  telemetry_=new QTextEdit;telemetry_->setReadOnly(true);telemetry_->setPlaceholderText("Robot telemetry will appear here when the firmware publishes supported telemetry frames.");telemetryLayout->addWidget(telemetry_);
  auto* actionsBox=new QGroupBox("System actions");auto* actionsLayout=new QVBoxLayout(actionsBox);
  auto* calibrate=new QPushButton("Request sensor calibration");auto* heartbeat=new QPushButton("Send heartbeat");auto* clear=new QPushButton("Clear event log");
  actionsLayout->addWidget(calibrate);actionsLayout->addWidget(heartbeat);actionsLayout->addWidget(clear);actionsLayout->addStretch();
  auto* safety=new QLabel("Safety interlock\\n\\n• Emergency stop sends a dedicated protocol frame.\\n• Disconnect does not guarantee motor shutdown.\\n• Firmware must enforce watchdog and motor-off defaults.\\n• Do not test with wheels on the ground until hardware safety is verified.");
  safety->setWordWrap(true);safety->setStyleSheet("background:#34231e;color:#f2d4c9;padding:14px;border-radius:8px");actionsLayout->addWidget(safety);
  grid->addWidget(telemetryBox,3);grid->addWidget(actionsBox,1);outer->addLayout(grid,1);
  setCentralWidget(root);statusBar()->showMessage("Desktop control station ready. No robot is connected.");
  serial_=std::make_unique<QSerialPort>();refresh_ports();
  connect(refresh_,&QPushButton::clicked,this,[this]{refresh_ports();});
  connect(connect_,&QPushButton::clicked,this,[this]{open_port();});
  connect(disconnect_,&QPushButton::clicked,this,[this]{close_port();});
  connect(start_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,1);});
  connect(pause_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,2);});
  connect(stop_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::MotorCommand,0);});
  connect(estop_,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::EmergencyStop,0);});
  connect(calibrate,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::Calibration,0x10);});
  connect(heartbeat,&QPushButton::clicked,this,[this]{send_action(oblivion::protocol::MessageType::Heartbeat,0);});
  connect(clear,&QPushButton::clicked,telemetry_,&QTextEdit::clear);
  connect(serial_.get(),&QSerialPort::readyRead,this,[this]{read_serial();});
  connect(serial_.get(),&QSerialPort::errorOccurred,this,[this](QSerialPort::SerialPortError e){if(e!=QSerialPort::NoError)log("Serial error: "+serial_->errorString());});
  refresh_timer_.setInterval(2000);connect(&refresh_timer_,&QTimer::timeout,this,[this]{if(!serial_->isOpen())refresh_ports();});refresh_timer_.start();
 }
 ~OblivionWindow()override{if(serial_&&serial_->isOpen())serial_->close();}
private:
 void log(const QString& s){telemetry_->append("["+QDateTime::currentDateTime().toString("HH:mm:ss.zzz")+"] "+s);}
 void refresh_ports(){const QString previous=port_->currentData().toString();port_->clear();for(const auto& p:QSerialPortInfo::availablePorts())port_->addItem(p.portName()+" — "+p.description(),p.portName());const int i=port_->findData(previous);if(i>=0)port_->setCurrentIndex(i);}
 void open_port(){
  if(port_->currentData().toString().isEmpty()){QMessageBox::warning(this,"No serial port","Connect the robot controller and select its serial port.");return;}
  serial_->setPortName(port_->currentData().toString());serial_->setBaudRate(QSerialPort::Baud115200);serial_->setDataBits(QSerialPort::Data8);serial_->setParity(QSerialPort::NoParity);serial_->setStopBits(QSerialPort::OneStop);serial_->setFlowControl(QSerialPort::NoFlowControl);
  if(!serial_->open(QIODevice::ReadWrite)){QMessageBox::critical(this,"Connection failed",serial_->errorString());return;}
  status_->setText("CONNECTED");status_->setStyleSheet("font-weight:700;color:#38a169;padding:8px");
  connect_->setEnabled(false);disconnect_->setEnabled(true);start_->setEnabled(true);pause_->setEnabled(true);stop_->setEnabled(true);estop_->setEnabled(true);port_->setEnabled(false);log("Serial connected: "+serial_->portName()+" at 115200 baud.");send_action(oblivion::protocol::MessageType::Heartbeat,0);
 }
 void close_port(){if(serial_->isOpen())serial_->close();status_->setText("DISCONNECTED");status_->setStyleSheet("font-weight:700;color:#d69e36;padding:8px");connect_->setEnabled(true);disconnect_->setEnabled(false);start_->setEnabled(false);pause_->setEnabled(false);stop_->setEnabled(false);estop_->setEnabled(false);port_->setEnabled(true);log("Serial connection closed. Firmware watchdog must independently stop motors.");}
 void send_action(oblivion::protocol::MessageType type,std::uint8_t action){
  if(!serial_->isOpen()){log("Action rejected: no robot connection.");return;}
  oblivion::protocol::Packet packet{};packet.type=type;packet.sequence=++sequence_;
  packet.payload={action,static_cast<std::uint8_t>(mode_->currentIndex()),static_cast<std::uint8_t>(controller_->currentIndex())};
  const auto bytes=oblivion::protocol::encode(packet);const auto written=serial_->write(reinterpret_cast<const char*>(bytes.data()),static_cast<qint64>(bytes.size()));
  if(written!=static_cast<qint64>(bytes.size())){log("Transport write failed: "+serial_->errorString());return;}
  const QString name=type==oblivion::protocol::MessageType::EmergencyStop?"EMERGENCY STOP":type==oblivion::protocol::MessageType::Heartbeat?"HEARTBEAT":type==oblivion::protocol::MessageType::SensorFrame?"CALIBRATION REQUEST":action==1?"START":action==2?"PAUSE":"STOP";
  log("TX "+name+" | seq="+QString::number(sequence_)+" | profile="+mode_->currentText()+" | controller="+controller_->currentText());
 }
 void read_serial(){
  receive_buffer_.append(serial_->readAll());
  while(receive_buffer_.size()>=12){
   const auto* data=reinterpret_cast<const std::uint8_t*>(receive_buffer_.constData());
   if(data[0]!=oblivion::protocol::kMagic0||data[1]!=oblivion::protocol::kMagic1){receive_buffer_.remove(0,1);continue;}
   const int payload=static_cast<int>(data[8])|(static_cast<int>(data[9])<<8);const int frame_size=12+payload;
   if(payload>512){receive_buffer_.remove(0,1);continue;}if(receive_buffer_.size()<frame_size)break;
   oblivion::protocol::Packet packet{};if(oblivion::protocol::decode(data,static_cast<std::size_t>(frame_size),packet)){
    log("RX frame type="+QString::number(static_cast<int>(packet.type))+" seq="+QString::number(packet.sequence)+" payload="+QString::number(packet.payload.size())+" bytes");
   }else log("RX invalid frame rejected (CRC/version/length).");
   receive_buffer_.remove(0,frame_size);
  }
 }
 QComboBox* port_{};QComboBox* mode_{};QComboBox* controller_{};QLabel* status_{};QTextEdit* telemetry_{};QPushButton* refresh_{};QPushButton* connect_{};QPushButton* disconnect_{};QPushButton* start_{};QPushButton* pause_{};QPushButton* stop_{};QPushButton* estop_{};
 std::unique_ptr<QSerialPort> serial_;QTimer refresh_timer_;QByteArray receive_buffer_;std::uint32_t sequence_{0};
};
int main(int argc,char** argv){QApplication app(argc,argv);QApplication::setApplicationName("OBLIVION Control Center");QApplication::setOrganizationName("DMJ");OblivionWindow window;window.show();return app.exec();}
