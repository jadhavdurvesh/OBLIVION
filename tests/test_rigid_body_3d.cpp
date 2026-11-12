#include "oblivion/simulation/rigid_body_3d.hpp"
#include <cmath>
#include <stdexcept>
void test_rigid_body_3d(){
 using namespace oblivion;
 RigidBody3DSimulator sim;
 if(!sim.configuration_valid())throw std::runtime_error("3D simulator defaults invalid");
 RigidBody3DState initial{};initial.pose.position_m.z=0.0219;
 sim.reset(initial);
 RigidBody3DInput input{};input.motors={0.0,0.0};
 RigidBody3DOutput out{};
 for(int i=0;i<1000;++i){out=sim.step(input,0.001);if(!out.valid)throw std::runtime_error("3D simulator integration failed");}
 if(!std::isfinite(out.state.pose.position_m.z)||out.state.pose.position_m.z<0.0)
  throw std::runtime_error("3D body penetrated terrain or diverged");
 if(out.contact_points==0)throw std::runtime_error("resting chassis has no terrain contact");
 RigidBody3DInput invalid{};invalid.motors.left=1.5;
 if(sim.step(invalid,0.001).valid)throw std::runtime_error("3D simulator accepted invalid motor command");
 RigidBody3DConfig bad{};bad.mass_kg=0;
 if(RigidBody3DSimulator(bad).configuration_valid())throw std::runtime_error("3D simulator accepted zero mass");
}
