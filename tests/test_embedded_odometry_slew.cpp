#include "oblivion/control/embedded_controller_bank.hpp"
#include "oblivion/localization/odometry.hpp"
#include "oblivion/safety/motor_slew_limiter.hpp"
#include <cmath>
#include <stdexcept>
namespace {
class TestControl final:public oblivion::IController{public:explicit TestControl(double x):x_(x){}oblivion::ControlOutput update(const oblivion::ControlInput&)noexcept override{return{x_,true};}void reset()noexcept override{}private:double x_;};
}
void test_embedded_odometry_slew(){
 using namespace oblivion;
 TestControl a(0.2),b(0.3);
 EmbeddedControllerSlot slots[]={{"a",&a,1.0},{"b",&b,1.0}};
 EmbeddedControllerBank bank;
 if(!bank.configure(slots,2,EmbeddedBankConfig{2,0.2,500}))throw std::runtime_error("embedded bank configuration failed");
 auto result=bank.evaluate({0.1,0.0,0.001},100);
 if(!result.valid||std::abs(result.command-0.25)>1e-9)throw std::runtime_error("embedded bank consensus incorrect");
 result=bank.evaluate({0.1,0.0,0.001},501);
 if(result.valid||!result.deadline_missed)throw std::runtime_error("embedded bank deadline gate failed");
 MotorSlewLimiter slew(2.0);
 const auto command=slew.apply({1.0,-1.0},0.1);
 if(std::abs(command.left-0.2)>1e-9||std::abs(command.right+0.2)>1e-9)throw std::runtime_error("motor slew limiter incorrect");
 DifferentialDriveOdometry odom({0.02,0.12,0.96});odom.reset();
 auto state=odom.update(0.01,0.01,0.0,0.01,false);
 if(!state.valid||std::abs(state.pose.x_m-0.01)>1e-9||std::abs(state.pose.y_m)>1e-9)throw std::runtime_error("odometry straight motion incorrect");
 state=odom.update(0.0,0.012,0.5,0.01,true);
 if(!state.valid||state.pose.heading_rad<=0)throw std::runtime_error("odometry gyro fusion incorrect");
}
