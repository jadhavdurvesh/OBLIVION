#include "oblivion/runtime/robot_control_runtime.hpp"
#include <array>
#include <cmath>
#include <stdexcept>
namespace {
class RuntimeFixedController final:public oblivion::IController{
public:oblivion::ControlOutput update(const oblivion::ControlInput&)noexcept override{return {0.1,true};}
void reset()noexcept override{}
};
}
void test_robot_control_runtime(){
 using namespace oblivion;
 ReflectanceConfig pc{};pc.sensor_count=5;pc.line_threshold=0.4;pc.minimum_total_strength=0.1;pc.line_is_dark=true;
 for(std::size_t i=0;i<5;++i){pc.positions_m[i]=(static_cast<double>(i)-2.0)*0.01;pc.dark_level[i]=0.0;pc.light_level[i]=1.0;}
 ReflectanceProcessor perception(pc);RuntimeFixedController controller;
 ParallelControllerBank bank({{"test",&controller,1.0}},ControllerBankConfig{std::chrono::milliseconds(50),1,0.5});
 MotorLimiter limiter(0.6);RobotControlRuntime runtime(perception,bank,limiter);
 std::array<double,5> raw{{1.0,1.0,0.0,1.0,1.0}};
 SensorFrame frame{};frame.timestamp_us=1000000;frame.reflectance=raw.data();frame.reflectance_count=raw.size();frame.battery_voltage=7.4;
 const auto first=runtime.process(frame,1000000);
 if(first.left!=0||first.right!=0)throw std::runtime_error("runtime must hold motors at zero on first sample");
 frame.timestamp_us=1005000;
 const auto second=runtime.process(frame,1005000);
 if(!runtime.diagnostics().safe||std::abs(second.left-0.15)>1e-9||std::abs(second.right-0.35)>1e-9)
  throw std::runtime_error("runtime failed sensor-to-controller-to-motor path");
 frame.timestamp_us=1010000;frame.battery_voltage=4.0;
 const auto low_battery=runtime.process(frame,1010000);
 if(low_battery.left!=0||low_battery.right!=0)throw std::runtime_error("runtime did not fail closed on low battery");
 runtime.emergency_stop();
 frame.timestamp_us=1015000;frame.battery_voltage=7.4;
 const auto stopped=runtime.process(frame,1015000);
 if(stopped.left!=0||stopped.right!=0)throw std::runtime_error("emergency stop did not latch");
}
