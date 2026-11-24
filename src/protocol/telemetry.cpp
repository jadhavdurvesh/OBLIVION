#include "oblivion/protocol/telemetry.hpp"
#include <cstddef>
#include <type_traits>
namespace oblivion::protocol {
namespace {
template<class T> void put(std::uint8_t*& p,T v) noexcept {
 using U=typename std::make_unsigned<T>::type;U u=static_cast<U>(v);
 for(std::size_t i=0;i<sizeof(T);++i)*p++=static_cast<std::uint8_t>((u>>(8u*i))&static_cast<U>(0xffu));
}
template<class T> T get(const std::uint8_t*& p) noexcept {
 using U=typename std::make_unsigned<T>::type;U u=0;
 for(std::size_t i=0;i<sizeof(T);++i)u|=static_cast<U>(*p++)<<(8u*i);
 return static_cast<T>(u);
}
}
bool encode_telemetry_v1(const TelemetryV1& v,std::uint8_t* out,std::size_t cap) noexcept {
 if(out==nullptr||cap<kTelemetryV1PayloadSize||v.line_confidence_permille>1000||v.cpu_load_percent>100)return false;
 auto* p=out;
 put<std::uint64_t>(p,v.timestamp_us);put<std::uint32_t>(p,v.validity);
 for(auto x:v.reflectance_raw)put<std::uint16_t>(p,x);
 put<std::int32_t>(p,v.line_position_um);put<std::uint16_t>(p,v.line_confidence_permille);
 put<std::int32_t>(p,v.left_encoder_ticks);put<std::int32_t>(p,v.right_encoder_ticks);
 put<std::int16_t>(p,v.gyro_x_mdps);put<std::int16_t>(p,v.gyro_y_mdps);put<std::int16_t>(p,v.gyro_z_mdps);
 put<std::int16_t>(p,v.accel_x_mg);put<std::int16_t>(p,v.accel_y_mg);put<std::int16_t>(p,v.accel_z_mg);
 put<std::int32_t>(p,v.linear_velocity_mmps);put<std::int32_t>(p,v.yaw_rate_mradps);put<std::int32_t>(p,v.curvature_milli_inv_m);
 put<std::uint16_t>(p,v.active_controller_mask);
 for(const auto& c:v.controllers){put<std::int16_t>(p,c.output_milli);put<std::uint16_t>(p,c.execution_time_us);put<std::uint8_t>(p,c.status);}
 put<std::uint8_t>(p,v.cpu_load_percent);put<std::uint32_t>(p,v.free_memory_bytes);put<std::uint16_t>(p,v.battery_millivolts);put<std::uint32_t>(p,v.fault_flags);
 return static_cast<std::size_t>(p-out)==kTelemetryV1PayloadSize;
}
bool decode_telemetry_v1(const std::uint8_t* in,std::size_t size,TelemetryV1& out) noexcept {
 if(in==nullptr||size!=kTelemetryV1PayloadSize)return false;
 TelemetryV1 v{};const auto* p=in;
 v.timestamp_us=get<std::uint64_t>(p);v.validity=get<std::uint32_t>(p);
 for(auto& x:v.reflectance_raw)x=get<std::uint16_t>(p);
 v.line_position_um=get<std::int32_t>(p);v.line_confidence_permille=get<std::uint16_t>(p);
 v.left_encoder_ticks=get<std::int32_t>(p);v.right_encoder_ticks=get<std::int32_t>(p);
 v.gyro_x_mdps=get<std::int16_t>(p);v.gyro_y_mdps=get<std::int16_t>(p);v.gyro_z_mdps=get<std::int16_t>(p);
 v.accel_x_mg=get<std::int16_t>(p);v.accel_y_mg=get<std::int16_t>(p);v.accel_z_mg=get<std::int16_t>(p);
 v.linear_velocity_mmps=get<std::int32_t>(p);v.yaw_rate_mradps=get<std::int32_t>(p);v.curvature_milli_inv_m=get<std::int32_t>(p);
 v.active_controller_mask=get<std::uint16_t>(p);
 for(auto& c:v.controllers){c.output_milli=get<std::int16_t>(p);c.execution_time_us=get<std::uint16_t>(p);c.status=get<std::uint8_t>(p);if(c.status>3)return false;}
 v.cpu_load_percent=get<std::uint8_t>(p);v.free_memory_bytes=get<std::uint32_t>(p);v.battery_millivolts=get<std::uint16_t>(p);v.fault_flags=get<std::uint32_t>(p);
 if(v.line_confidence_permille>1000||v.cpu_load_percent>100)return false;
 out=v;return true;
}
} // namespace oblivion::protocol
