#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace oblivion {

constexpr std::size_t kMaxReflectanceSensors = 32;

struct ReflectanceConfig {
    std::array<double, kMaxReflectanceSensors> positions_m{};
    std::array<double, kMaxReflectanceSensors> dark_level{};
    std::array<double, kMaxReflectanceSensors> light_level{};
    std::size_t sensor_count{0};
    double line_threshold{0.55};
    double minimum_total_strength{0.60};
    bool line_is_dark{true};
};

struct LineObservation {
    std::uint64_t timestamp_us{0};
    double position_m{0.0};
    double width_m{0.0};
    double confidence{0.0};
    bool detected{false};
    bool input_valid{false};
};

class ReflectanceProcessor {
public:
    explicit ReflectanceProcessor(ReflectanceConfig config) noexcept;
    [[nodiscard]] bool configuration_valid() const noexcept;
    [[nodiscard]] LineObservation process(const double* raw_values,
                                          std::size_t count,
                                          std::uint64_t timestamp_us) const noexcept;
private:
    ReflectanceConfig config_;
};

} // namespace oblivion
