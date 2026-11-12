#include "oblivion/control/pid_controller.hpp"
#include <cmath>
#include <stdexcept>
namespace { void require(bool c,const char* m){if(!c)throw std::runtime_error(m);} }
void test_pid() {
    using namespace oblivion;
    PidController pid({2.0,0.5,0.1},{-0.75,0.75,-0.2,0.2});
    const ControlOutput first=pid.update({0.5,0.0,0.01});
    require(first.valid,"valid PID input rejected");
    require(first.command<=0.75 && first.command>=-0.75,"PID output not clamped");
    require(!pid.update({0.5,0.0,0.0}).valid,"zero dt must be rejected");
    PidController derivative_only({0.0, 0.0, 1.0}, {-0.75, 0.75, -0.2, 0.2});
    const ControlOutput derivative = derivative_only.update({0.0, 0.5, 0.01});
    require(derivative.valid && std::abs(derivative.command + 0.5) < 1e-12,
            "derivative-on-measurement response incorrect");

    pid.reset();
    const ControlOutput after=pid.update({-0.1,0.0,0.01});
    require(after.valid && std::isfinite(after.command),"PID failed after reset");
}
