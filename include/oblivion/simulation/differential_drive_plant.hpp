#pragma once
#include "oblivion/core/types.hpp"
#include <cstdint>

namespace oblivion {

struct PlantConfig {
    double wheel_radius_m{0.02};
    double track_width_m{0.12};
    double maximum_wheel_speed_mps{1.5};
    double maximum_wheel_acceleration_mps2{8.0};
    double motor_time_constant_seconds{0.04};
    double traction_factor{1.0};
};

class DifferentialDrivePlant {
public:
    explicit DifferentialDrivePlant(PlantConfig config) noexcept;
    [[nodiscard]] bool configuration_valid() const noexcept;
    bool step(const MotorCommand& normalized_motor_command, double dt_seconds) noexcept;
    [[nodiscard]] const Pose2D& pose() const noexcept;
    [[nodiscard]] WheelSpeeds wheel_speeds() const noexcept;
    void reset(const Pose2D& pose = {}) noexcept;
private:
    PlantConfig config_;
    Pose2D pose_{};
    WheelSpeeds wheels_{};
};

} // namespace oblivion
