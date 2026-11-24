#include "oblivion/protocol/telemetry.hpp"
#include <array>
#include <stdexcept>
void test_telemetry_codec(){
 using namespace oblivion::protocol;
 TelemetryV1 in{};in.timestamp_us=987654321;in.validity=ValidReflectance|ValidMotion|ValidControllerStats;
 for(std::size_t i=0;i<in.reflectance_raw.size();++i)in.reflectance_raw[i]=static_cast<std::uint16_t>(100+i*100);
 in.line_position_um=-1250;in.line_confidence_permille=925;
 in.left_encoder_ticks=-123456;in.right_encoder_ticks=654321;
 in.gyro_z_mdps=-2500;in.accel_y_mg=1000;in.linear_velocity_mmps=850;
 in.yaw_rate_mradps=-120;in.curvature_milli_inv_m=450;
 in.active_controller_mask=0x023f;in.controllers[0]={125,100,1};in.controllers[1]={-250,420,1};
 in.cpu_load_percent=42;in.free_memory_bytes=32768;in.battery_millivolts=7400;in.fault_flags=0x10;
 std::array<std::uint8_t,kTelemetryV1PayloadSize> bytes{};
 if(!encode_telemetry_v1(in,bytes.data(),bytes.size()))throw std::runtime_error("telemetry encode failed");
 TelemetryV1 out{};
 if(!decode_telemetry_v1(bytes.data(),bytes.size(),out))throw std::runtime_error("telemetry decode failed");
 if(out.timestamp_us!=in.timestamp_us||out.line_position_um!=in.line_position_um||out.left_encoder_ticks!=in.left_encoder_ticks||out.right_encoder_ticks!=in.right_encoder_ticks||out.controllers[1].execution_time_us!=420||out.battery_millivolts!=7400)throw std::runtime_error("telemetry round-trip mismatch");
 if(decode_telemetry_v1(bytes.data(),bytes.size()-1,out))throw std::runtime_error("telemetry accepted truncated payload");
 in.cpu_load_percent=101;
 if(encode_telemetry_v1(in,bytes.data(),bytes.size()))throw std::runtime_error("telemetry accepted invalid CPU percentage");
}
