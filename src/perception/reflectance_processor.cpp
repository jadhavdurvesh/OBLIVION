#include "oblivion/perception/reflectance_processor.hpp"
#include <algorithm>
#include <cmath>

namespace oblivion {

ReflectanceProcessor::ReflectanceProcessor(ReflectanceConfig config) noexcept
    : config_(config) {}

bool ReflectanceProcessor::configuration_valid() const noexcept {
    if (config_.sensor_count < 2 || config_.sensor_count > kMaxReflectanceSensors ||
        !std::isfinite(config_.line_threshold) || config_.line_threshold <= 0.0 ||
        config_.line_threshold > 1.0 || !std::isfinite(config_.minimum_total_strength) ||
        config_.minimum_total_strength <= 0.0) return false;

    for (std::size_t i = 0; i < config_.sensor_count; ++i) {
        if (!std::isfinite(config_.positions_m[i]) ||
            !std::isfinite(config_.dark_level[i]) ||
            !std::isfinite(config_.light_level[i]) ||
            config_.light_level[i] <= config_.dark_level[i]) return false;
        if (i > 0 && config_.positions_m[i] <= config_.positions_m[i - 1]) return false;
    }
    return true;
}

LineObservation ReflectanceProcessor::process(const double* raw_values,
                                               std::size_t count,
                                               std::uint64_t timestamp_us) const noexcept {
    LineObservation result{};
    result.timestamp_us = timestamp_us;
    if (!configuration_valid() || raw_values == nullptr || count != config_.sensor_count) return result;

    double weighted_position = 0.0;
    double total_strength = 0.0;
    double peak_strength = 0.0;
    double first_active = 0.0;
    double last_active = 0.0;
    bool has_active = false;

    for (std::size_t i = 0; i < count; ++i) {
        if (!std::isfinite(raw_values[i])) return result;
        const double span = config_.light_level[i] - config_.dark_level[i];
        const double normalized = std::clamp(
            (raw_values[i] - config_.dark_level[i]) / span, 0.0, 1.0);
        const double strength = config_.line_is_dark ? 1.0 - normalized : normalized;

        weighted_position += strength * config_.positions_m[i];
        total_strength += strength;
        peak_strength = std::max(peak_strength, strength);

        if (strength >= config_.line_threshold) {
            if (!has_active) first_active = config_.positions_m[i];
            last_active = config_.positions_m[i];
            has_active = true;
        }
    }

    result.input_valid = true;
    if (total_strength < config_.minimum_total_strength || total_strength <= 0.0) return result;

    result.position_m = weighted_position / total_strength;
    result.confidence = std::clamp(
        peak_strength * std::min(1.0, total_strength / config_.minimum_total_strength),
        0.0, 1.0);
    result.detected = true;

    if (has_active) {
        double spacing = (config_.positions_m[count - 1] - config_.positions_m[0]) /
                         static_cast<double>(count - 1);
        result.width_m = (last_active - first_active) + spacing;
    }
    return result;
}

} // namespace oblivion
