#include "oblivion/motion/dynamics.hpp"
#include <cmath>
#include <stdexcept>
namespace {void require(bool c,const char*m){if(!c)throw std::runtime_error(m);}}
void test_dynamics(){
 using namespace oblivion;
 DynamicsParameters p{};p.mass_kg=1.0;p.yaw_inertia_kg_m2=0.02;p.wheelbase_track_m=0.2;
 p.friction_coefficient=0.8;p.rolling_resistance_coefficient=0;p.linear_drag_coefficient=0;p.angular_damping_n_m_per_radps=0;
 DifferentialDriveDynamics model(p);require(model.parameters_valid(),"valid dynamics rejected");
 DynamicsState s{};DynamicsInput forward{};forward.left_wheel_force_n=1.0;forward.right_wheel_force_n=1.0;
 const auto straight=model.step(s,forward,0.01);require(straight.valid,"straight dynamics failed");
 require(straight.state.forward_speed_mps>0,"drive forces did not accelerate robot");
 require(std::abs(straight.state.yaw_rate_radps)<1e-12,"symmetric force generated yaw");
 DynamicsInput turn{};turn.left_wheel_force_n=-0.2;turn.right_wheel_force_n=0.2;
 const auto turning=model.step(s,turn,0.01);require(turning.valid,"turn dynamics failed");
 require(turning.state.yaw_rate_radps>0,"differential wheel force did not create yaw");
 DynamicsState fast{};fast.forward_speed_mps=4.0;fast.yaw_rate_radps=3.0;
 const auto corner=model.step(fast,{},0.01);
 require(corner.valid&&corner.traction_limited,"high lateral acceleration did not trigger traction limit");
 DynamicsInput hill{};hill.road_grade_rad=0.1;
 const auto uphill=model.step(s,hill,0.01);require(uphill.valid,"slope dynamics failed");
 require(uphill.state.forward_speed_mps<0,"gravity did not oppose uphill movement from rest");
 require(!model.step(s,{},0.0).valid,"zero timestep accepted");
}
