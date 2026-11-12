#pragma once
#include <cstdint>
namespace oblivion {
struct Pose2D { double x_m{0.0}; double y_m{0.0}; double heading_rad{0.0}; };
struct Twist2D { double linear_mps{0.0}; double angular_radps{0.0}; };
struct WheelSpeeds { double left_mps{0.0}; double right_mps{0.0}; };
struct MotorCommand { double left{0.0}; double right{0.0}; };
struct SensorFrame {
    std::uint64_t timestamp_us{0};
    const double* reflectance{nullptr};
    std::uint32_t reflectance_count{0};
    double left_encoder_delta{0.0};
    double right_encoder_delta{0.0};
    double gyro_z_radps{0.0};
    double battery_voltage{0.0};
    bool imu_valid{false};
    bool encoders_valid{false};
};
} // namespace oblivion
