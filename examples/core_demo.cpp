#include "oblivion/control/pid_controller.hpp"
#include "oblivion/kinematics/differential_drive.hpp"
#include "oblivion/safety/motor_limiter.hpp"
#include <iostream>
int main() {
    using namespace oblivion;
    const DifferentialDriveModel robot({0.02, 0.12});
    const Twist2D requested{0.35, 0.8};
    const WheelSpeeds wheels = robot.inverse(requested);
    const Twist2D reconstructed = robot.forward(wheels);
    PidController steering({1.8, 0.05, 0.02}, {-1.0, 1.0, -0.4, 0.4});
    const ControlOutput output = steering.update({0.12, 0.0, 0.001});
    const MotorCommand safe = MotorLimiter(1.0).apply({0.4 - output.command, 0.4 + output.command});
    std::cout << "OBLIVION core demonstration\n"
              << "linear_mps=" << reconstructed.linear_mps << " angular_radps=" << reconstructed.angular_radps << '\n'
              << "motor_left=" << safe.left << " motor_right=" << safe.right << '\n';
    return output.valid ? 0 : 1;
}
