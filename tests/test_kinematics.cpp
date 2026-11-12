#include "oblivion/kinematics/differential_drive.hpp"
#include <cmath>
#include <stdexcept>
namespace { void require(bool c, const char* m) { if (!c) throw std::runtime_error(m); } bool near(double a,double b,double e=1e-9){return std::abs(a-b)<=e;} }
void test_kinematics() {
    using namespace oblivion;
    DifferentialDriveModel model({0.02,0.12});
    const Twist2D desired{0.5,1.0};
    const Twist2D actual=model.forward(model.inverse(desired));
    require(near(actual.linear_mps,desired.linear_mps),"inverse/forward linear mismatch");
    require(near(actual.angular_radps,desired.angular_radps),"inverse/forward angular mismatch");
    const Pose2D moved=model.integrate({}, {0.2,0.2}, 0.5);
    require(near(moved.x_m,0.1),"straight integration x mismatch");
    require(near(moved.y_m,0.0),"straight integration y mismatch");
    require(near(moved.heading_rad,0.0),"straight integration heading mismatch");
    require(near(model.integrate(moved,{0.2,0.2},0.0).x_m,moved.x_m),"non-positive dt must not integrate");
}
