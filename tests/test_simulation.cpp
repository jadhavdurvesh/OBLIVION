#include "oblivion/simulation/differential_drive_plant.hpp"
#include <cmath>
#include <stdexcept>
namespace {void require(bool c,const char*m){if(!c)throw std::runtime_error(m);}}
void test_simulation(){
 using namespace oblivion;
 DifferentialDrivePlant plant({0.02,0.12,1.0,10.0,0.02,1.0});
 require(plant.configuration_valid(),"valid plant rejected");
 for(int i=0;i<100;++i)require(plant.step({0.5,0.5},0.01),"plant step failed");
 require(plant.pose().x_m>0.0,"plant did not move forward");
 require(std::abs(plant.pose().y_m)<1e-9,"straight plant drifted laterally");
 require(!plant.step({2.0,0.0},0.01),"plant accepted unbounded motor command");
 require(!plant.step({0.0,0.0},0.0),"plant accepted invalid timestep");
 plant.reset(); require(plant.pose().x_m==0.0,"plant reset failed");
}
