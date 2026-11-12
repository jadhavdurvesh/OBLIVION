#include "oblivion/safety/motor_limiter.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
namespace { void require(bool c,const char* m){if(!c)throw std::runtime_error(m);} }
void test_safety() {
    using namespace oblivion;
    const MotorCommand bounded=MotorLimiter(0.6).apply({1.2,-0.9});
    require(std::abs(bounded.left-0.6)<1e-12,"left motor limit failed");
    require(std::abs(bounded.right+0.6)<1e-12,"right motor limit failed");
    const MotorCommand invalid=MotorLimiter(0.6).apply({std::numeric_limits<double>::quiet_NaN(),0.2});
    require(invalid.left==0.0 && invalid.right==0.0,"non-finite command must fail closed");
    require(!MotorLimiter(-1.0).configuration_valid(),"negative limit must be invalid");
}
