#pragma once
#include "oblivion/core/types.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace oblivion {
constexpr std::size_t kMaxTrackPoints=512;
struct TrackPoint { double x_m{0.0}; double y_m{0.0}; };
struct Track {
 std::array<TrackPoint,kMaxTrackPoints> centerline{};
 std::size_t point_count{0};
 double line_width_m{0.012};
 [[nodiscard]] bool valid()const noexcept;
 [[nodiscard]] double distance_to_centerline(TrackPoint p)const noexcept;
};
class TrackBuilder {
public:
 explicit TrackBuilder(double line_width_m=0.012)noexcept;
 bool add_straight(TrackPoint start,TrackPoint end)noexcept;
 bool add_arc(TrackPoint center,double radius_m,double start_angle_rad,double sweep_rad,std::size_t samples)noexcept;
 [[nodiscard]] Track build()const noexcept;
private:
 Track track_{};
 bool append(TrackPoint p)noexcept;
};
struct SimulatedReflectanceConfig {
 std::array<double,32> lateral_positions_m{};
 std::size_t sensor_count{0};
 double white_level{1.0};
 double dark_level{0.0};
 double noise_amplitude{0.0};
 std::uint32_t random_seed{0x0B11A10U};
};
class ReflectanceArraySimulator {
public:
 explicit ReflectanceArraySimulator(SimulatedReflectanceConfig config)noexcept;
 [[nodiscard]] bool configuration_valid()const noexcept;
 bool sample(const Track&,const Pose2D&,std::uint64_t,double*,std::size_t)noexcept;
private:
 SimulatedReflectanceConfig config_;
 std::uint32_t random_state_;
 double noise()noexcept;
};
} // namespace oblivion
