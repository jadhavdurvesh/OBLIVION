#include "oblivion/perception/reflectance_processor.hpp"
#include <cmath>
#include <stdexcept>
namespace { void require(bool c, const char* m) { if (!c) throw std::runtime_error(m); } }
void test_perception() {
    using namespace oblivion;
    ReflectanceConfig cfg{};
    cfg.sensor_count = 5;
    cfg.line_threshold = 0.5;
    cfg.minimum_total_strength = 0.5;
    cfg.line_is_dark = true;
    for (std::size_t i = 0; i < cfg.sensor_count; ++i) {
        cfg.positions_m[i] = (static_cast<double>(i) - 2.0) * 0.01;
        cfg.dark_level[i] = 0.0;
        cfg.light_level[i] = 1.0;
    }
    ReflectanceProcessor processor(cfg);
    require(processor.configuration_valid(), "valid reflectance config rejected");

    const double centered[] = {1.0, 0.8, 0.0, 0.8, 1.0};
    const LineObservation line = processor.process(centered, 5, 1234);
    require(line.input_valid && line.detected, "center line not detected");
    require(std::abs(line.position_m) < 1e-9, "center line position incorrect");
    require(line.confidence > 0.9, "center line confidence too low");

    const double absent[] = {1.0, 1.0, 1.0, 1.0, 1.0};
    const LineObservation lost = processor.process(absent, 5, 2345);
    require(lost.input_valid && !lost.detected, "absent line should be reported lost");

    const double invalid[] = {1.0, 1.0, NAN, 1.0, 1.0};
    require(!processor.process(invalid, 5, 3456).input_valid,
            "non-finite sensor value must invalidate frame");
}
