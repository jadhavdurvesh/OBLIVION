#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
namespace oblivion::protocol {
constexpr std::uint8_t kTelemetrySchemaVersion=1;
constexpr std::size_t kReflectanceChannels=8;
constexpr std::size_t kControllerSlots=11;
enum TelemetryValidity : std::uint32_t {
    ValidReflectance=1u<<0, ValidLineEstimate=1u<<1, ValidEncoders=1u<<2,
    ValidImu=1u<<3, ValidMotion=1u<<4, ValidControllerStats=1u<<5,
    ValidSystemHealth=1u<<6, ValidPower=1u<<7
};
struct ControllerTelemetry {
    std::int16_t output_milli{0};
    std::uint16_t execution_time_us{0};
    std::uint8_t status{0}; // 0=not scheduled, 1=valid, 2=invalid, 3=deadline miss
};
struct TelemetryV1 {
    std::uint64_t timestamp_us{0};
    std::uint32_t validity{0};
    std::array<std::uint16_t,kReflectanceChannels> reflectance_raw{};
    std::int32_t line_position_um{0};
    std::uint16_t line_confidence_permille{0};
    std::int32_t left_encoder_ticks{0},right_encoder_ticks{0};
    std::int16_t gyro_x_mdps{0},gyro_y_mdps{0},gyro_z_mdps{0};
    std::int16_t accel_x_mg{0},accel_y_mg{0},accel_z_mg{0};
    std::int32_t linear_velocity_mmps{0};
    std::int32_t yaw_rate_mradps{0};
    std::int32_t curvature_milli_inv_m{0};
    std::uint16_t active_controller_mask{0};
    std::array<ControllerTelemetry,kControllerSlots> controllers{};
    std::uint8_t cpu_load_percent{0};
    std::uint32_t free_memory_bytes{0};
    std::uint16_t battery_millivolts{0};
    std::uint32_t fault_flags{0};
};
constexpr std::size_t kTelemetryV1PayloadSize=8+4+16+4+2+8+12+12+12+2+(kControllerSlots*5)+1+4+2+4;
bool encode_telemetry_v1(const TelemetryV1& value,std::uint8_t* output,std::size_t capacity) noexcept;
bool decode_telemetry_v1(const std::uint8_t* input,std::size_t size,TelemetryV1& value) noexcept;
} // namespace oblivion::protocol
