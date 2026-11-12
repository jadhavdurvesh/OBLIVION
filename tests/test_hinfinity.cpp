#include "oblivion/control/h_infinity_controller.hpp"
#include <cmath>
#include <stdexcept>
namespace { void require(bool c,const char*m){if(!c)throw std::runtime_error(m);} }
void test_hinfinity(){
 using namespace oblivion;
 HInfinityController h({2.0,0.5,{-0.7,0.7},10.0});
 const auto good=h.update({0.2,0.1,0.01});
 require(good.valid,"H-infinity runtime rejected valid state");
 require(good.command>=-0.7&&good.command<=0.7,"H-infinity output not bounded");
 require(!h.update({11.0,0.0,0.01}).valid,"H-infinity accepted state outside validated envelope");
 require(!h.update({0.0,0.0,0.0}).valid,"H-infinity accepted invalid timestep");
}
