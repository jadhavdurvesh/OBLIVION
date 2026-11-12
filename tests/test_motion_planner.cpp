#include "oblivion/motion/planner.hpp"
#include <cmath>
#include <stdexcept>
namespace {void require(bool c,const char*m){if(!c)throw std::runtime_error(m);}}
void test_motion_planner(){
 using namespace oblivion;
 MotionConstraints c{};c.maximum_speed_mps=2.0;c.maximum_wheel_speed_mps=2.0;
 c.maximum_lateral_acceleration_mps2=2.0;c.maximum_yaw_rate_radps=5.0;
 MotionPlanner planner(c);require(planner.constraints_valid(),"planner config invalid");
 const double k=planner.curvature({0,0},{1,0},{1,1});
 require(k>0.0,"left turn curvature sign incorrect");
 const double v=planner.maximum_corner_speed(k);
 require(v>0.0&&v<2.0,"corner speed not constrained");
 const auto wheels=planner.wheel_speeds(1.0,k);
 require(wheels.left_mps<wheels.right_mps,"left turn did not slow inner wheel");
 const double braking=planner.turn_anticipation_distance(1.0,2.0);
 require(braking>0.25&&braking<0.4,"turn anticipation distance incorrect");
 PathSample path[5]{{{0,0},0,0},{{0.5,0},0,0},{{1,0},0,0},{{1,0.5},0,0},{{1,1},0,0}};
 const auto plan=planner.plan(path,5,0.0);
 require(plan.valid&&plan.count==5,"trajectory planning failed");
 require(plan.points.back().speed_mps==0.0,"terminal stop constraint not applied");
 require(plan.points[2].left_wheel_speed_mps<plan.points[2].right_wheel_speed_mps,
         "planned left-turn wheel speeds incorrect");
 require(plan.estimated_time_seconds>0,"trajectory time estimate missing");
}
