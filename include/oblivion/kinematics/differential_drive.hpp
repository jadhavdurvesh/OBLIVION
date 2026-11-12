#pragma once
#include "oblivion/core/types.hpp"
namespace oblivion {
struct DifferentialDriveGeometry { double wheel_radius_m{0.02}; double track_width_m{0.12}; };
class DifferentialDriveModel {
public:
    explicit DifferentialDriveModel(DifferentialDriveGeometry geometry);
    [[nodiscard]] const DifferentialDriveGeometry& geometry() const noexcept;
    [[nodiscard]] Twist2D forward(const WheelSpeeds& wheels) const noexcept;
    [[nodiscard]] WheelSpeeds inverse(const Twist2D& twist) const noexcept;
    [[nodiscard]] Pose2D integrate(const Pose2D& pose, const WheelSpeeds& wheels, double dt_seconds) const noexcept;
private:
    DifferentialDriveGeometry geometry_;
};
} // namespace oblivion
