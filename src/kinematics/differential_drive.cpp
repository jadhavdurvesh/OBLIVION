#include "oblivion/kinematics/differential_drive.hpp"
#include <cmath>
#include <stdexcept>
namespace oblivion {
DifferentialDriveModel::DifferentialDriveModel(DifferentialDriveGeometry geometry) : geometry_(geometry) {
    if (!std::isfinite(geometry_.wheel_radius_m) || !std::isfinite(geometry_.track_width_m) ||
        geometry_.wheel_radius_m <= 0.0 || geometry_.track_width_m <= 0.0) {
        throw std::invalid_argument("Differential-drive dimensions must be finite and positive");
    }
}
const DifferentialDriveGeometry& DifferentialDriveModel::geometry() const noexcept { return geometry_; }
Twist2D DifferentialDriveModel::forward(const WheelSpeeds& wheels) const noexcept {
    return {(wheels.left_mps + wheels.right_mps) * 0.5,
            (wheels.right_mps - wheels.left_mps) / geometry_.track_width_m};
}
WheelSpeeds DifferentialDriveModel::inverse(const Twist2D& twist) const noexcept {
    const double half_track_omega = 0.5 * geometry_.track_width_m * twist.angular_radps;
    return {twist.linear_mps - half_track_omega, twist.linear_mps + half_track_omega};
}
Pose2D DifferentialDriveModel::integrate(const Pose2D& pose, const WheelSpeeds& wheels, double dt_seconds) const noexcept {
    if (!std::isfinite(dt_seconds) || dt_seconds <= 0.0) return pose;
    const Twist2D body = forward(wheels);
    const double mid_heading = pose.heading_rad + 0.5 * body.angular_radps * dt_seconds;
    Pose2D next = pose;
    next.x_m += body.linear_mps * std::cos(mid_heading) * dt_seconds;
    next.y_m += body.linear_mps * std::sin(mid_heading) * dt_seconds;
    next.heading_rad += body.angular_radps * dt_seconds;
    return next;
}
} // namespace oblivion
