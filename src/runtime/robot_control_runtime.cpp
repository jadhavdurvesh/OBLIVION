#include "oblivion/runtime/robot_control_runtime.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace oblivion {
RobotControlRuntime::RobotControlRuntime(ReflectanceProcessor p,ParallelControllerBank& c,MotorLimiter l,ControlRuntimeConfig cfg)
 :perception_(std::move(p)),controllers_(c),limiter_(l),config_(cfg){
 if(!perception_.configuration_valid()||!limiter_.configuration_valid()||
 !std::isfinite(cfg.cruise_command)||cfg.cruise_command<0||cfg.cruise_command>1||
 !std::isfinite(cfg.minimum_battery_voltage)||cfg.minimum_battery_voltage<0||
 !std::isfinite(cfg.maximum_sensor_age_seconds)||cfg.maximum_sensor_age_seconds<=0||
 !std::isfinite(cfg.maximum_line_confidence)||cfg.maximum_line_confidence<0||cfg.maximum_line_confidence>1||
 !std::isfinite(cfg.maximum_steering)||cfg.maximum_steering<=0||cfg.maximum_steering>1)
 throw std::invalid_argument("invalid robot control runtime configuration");
}
MotorCommand RobotControlRuntime::process(const SensorFrame& frame,std::uint64_t now_us) noexcept{
 diagnostics_={};
 if(emergency_stopped_)return {};
 if(frame.timestamp_us==0||frame.timestamp_us>now_us||frame.reflectance==nullptr||
 frame.reflectance_count<2||!std::isfinite(frame.battery_voltage)||
 frame.battery_voltage<config_.minimum_battery_voltage)return {};
 const double age=static_cast<double>(now_us-frame.timestamp_us)*1e-6;
 if(age>config_.maximum_sensor_age_seconds)return {};
 diagnostics_.frame_valid=true;diagnostics_.battery_valid=true;
 diagnostics_.line=perception_.process(frame.reflectance,frame.reflectance_count,frame.timestamp_us);
 diagnostics_.line_valid=diagnostics_.line.input_valid&&diagnostics_.line.detected&&
 diagnostics_.line.confidence>=config_.maximum_line_confidence;
 if(!diagnostics_.line_valid)return {};
 if(!initialized_){
  previous_timestamp_us_=frame.timestamp_us;previous_line_position_m_=diagnostics_.line.position_m;
  initialized_=true;return {};
 }
 if(frame.timestamp_us<=previous_timestamp_us_)return {};
 const double dt=static_cast<double>(frame.timestamp_us-previous_timestamp_us_)*1e-6;
 if(!std::isfinite(dt)||dt<0.0001||dt>0.02)return {};
 diagnostics_.dt_seconds=dt;
 diagnostics_.line_rate_mps=(diagnostics_.line.position_m-previous_line_position_m_)/dt;
 if(!std::isfinite(diagnostics_.line_rate_mps))return {};
 const ControlInput input{diagnostics_.line.position_m,diagnostics_.line_rate_mps,dt};
 diagnostics_.consensus=controllers_.evaluate(input);
 if(!diagnostics_.consensus.valid||diagnostics_.consensus.deadline_missed||
 diagnostics_.consensus.disagreement_exceeded)return {};
 const double steering=std::clamp(diagnostics_.consensus.command,-config_.maximum_steering,config_.maximum_steering);
 diagnostics_.requested={config_.cruise_command-steering,config_.cruise_command+steering};
 diagnostics_.applied=limiter_.apply(diagnostics_.requested);
 diagnostics_.safe=true;
 previous_timestamp_us_=frame.timestamp_us;previous_line_position_m_=diagnostics_.line.position_m;
 return diagnostics_.applied;
}
void RobotControlRuntime::emergency_stop() noexcept{emergency_stopped_=true;diagnostics_.safe=false;diagnostics_.applied={};}
const RuntimeDiagnostics& RobotControlRuntime::diagnostics()const noexcept{return diagnostics_;}
} // namespace oblivion
