#include "oblivion/simulation/rigid_body_3d.hpp"
#include <algorithm>
#include <cmath>
namespace oblivion {
namespace {
constexpr double eps=1e-10;
Vector3 add(Vector3 a,Vector3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vector3 sub(Vector3 a,Vector3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vector3 mul(Vector3 a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(Vector3 a,Vector3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vector3 cross(Vector3 a,Vector3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double norm(Vector3 a){return std::sqrt(dot(a,a));}
bool finite(Vector3 a){return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z);}
Quaternion normalize(Quaternion q){
 const double n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
 if(!std::isfinite(n)||n<eps)return {};
 return {q.w/n,q.x/n,q.y/n,q.z/n};
}
Vector3 rotate(Quaternion q,Vector3 v){
 const Vector3 u{q.x,q.y,q.z};
 return add(v,add(mul(cross(u,v),2.0*q.w),mul(cross(u,cross(u,v)),2.0)));
}
Vector3 inverse_rotate(Quaternion q,Vector3 v){return rotate({q.w,-q.x,-q.y,-q.z},v);}
Quaternion integrate(Quaternion q,Vector3 omega,double dt){
 const Quaternion dq{0.0,omega.x,omega.y,omega.z};
 Quaternion out{
  q.w+0.5*(-dq.x*q.x-dq.y*q.y-dq.z*q.z)*dt,
  q.x+0.5*( dq.x*q.w+dq.y*q.z-dq.z*q.y)*dt,
  q.y+0.5*(-dq.x*q.z+dq.y*q.w+dq.z*q.x)*dt,
  q.z+0.5*( dq.x*q.y-dq.y*q.x+dq.z*q.w)*dt};
 return normalize(out);
}
}
RigidBody3DSimulator::RigidBody3DSimulator(RigidBody3DConfig c)noexcept:config_(c){}
bool RigidBody3DSimulator::configuration_valid()const noexcept{
 const auto&c=config_;
 return std::isfinite(c.mass_kg)&&c.mass_kg>0&&finite(c.inertia_kg_m2)&&c.inertia_kg_m2.x>0&&c.inertia_kg_m2.y>0&&c.inertia_kg_m2.z>0&&
 finite(c.chassis_size_m)&&c.chassis_size_m.x>0&&c.chassis_size_m.y>0&&c.chassis_size_m.z>0&&
 std::isfinite(c.wheel_radius_m)&&c.wheel_radius_m>0&&std::isfinite(c.wheel_track_m)&&c.wheel_track_m>0&&
 std::isfinite(c.wheelbase_m)&&c.wheelbase_m>0&&std::isfinite(c.wheel_inertia_kg_m2)&&c.wheel_inertia_kg_m2>0&&
 std::isfinite(c.maximum_motor_torque_nm)&&c.maximum_motor_torque_nm>0&&std::isfinite(c.motor_time_constant_s)&&c.motor_time_constant_s>=0&&
 std::isfinite(c.tire_longitudinal_stiffness_n_per_mps)&&c.tire_longitudinal_stiffness_n_per_mps>0&&
 std::isfinite(c.tire_lateral_stiffness_n_per_mps)&&c.tire_lateral_stiffness_n_per_mps>0&&
 std::isfinite(c.friction_coefficient)&&c.friction_coefficient>0&&
 std::isfinite(c.suspension_stiffness_n_per_m)&&c.suspension_stiffness_n_per_m>0&&
 std::isfinite(c.suspension_damping_n_s_per_m)&&c.suspension_damping_n_s_per_m>=0&&
 std::isfinite(c.rolling_resistance)&&c.rolling_resistance>=0&&
 std::isfinite(c.aerodynamic_drag_n_per_mps2)&&c.aerodynamic_drag_n_per_mps2>=0&&
 std::isfinite(c.gravity_mps2)&&c.gravity_mps2>0;
}
const RigidBody3DState& RigidBody3DSimulator::state()const noexcept{return state_;}
void RigidBody3DSimulator::reset(const RigidBody3DState&s)noexcept{state_=s;state_.pose.orientation=normalize(state_.pose.orientation);}
RigidBody3DOutput RigidBody3DSimulator::step(const RigidBody3DInput&in,double dt)noexcept{
 RigidBody3DOutput out{};out.state=state_;
 if(!configuration_valid()||!std::isfinite(dt)||dt<=0||dt>0.02||
    !std::isfinite(in.motors.left)||!std::isfinite(in.motors.right)||std::abs(in.motors.left)>1||
    std::abs(in.motors.right)>1||!std::isfinite(in.battery_voltage_ratio)||in.battery_voltage_ratio<0||
    in.battery_voltage_ratio>1||!finite(in.terrain.point_m)||!finite(in.terrain.normal)||
    std::abs(norm(in.terrain.normal)-1.0)>1e-5)return out;
 const auto q=normalize(state_.pose.orientation);
 if(!finite(state_.pose.position_m)||!finite(state_.linear_velocity_world_mps)||
    !finite(state_.angular_velocity_body_radps))return out;
 Vector3 force{0,0,-config_.mass_kg*config_.gravity_mps2};
 Vector3 torque{};
 const double half_x=config_.wheelbase_m*0.5,half_y=config_.wheel_track_m*0.5;
 const double half_z=-config_.chassis_size_m.z*0.5;
 const double corner_x[4]={-half_x,-half_x,half_x,half_x};
 const double corner_y[4]={-half_y,half_y,-half_y,half_y};
 double normal_left=0,normal_right=0;
 for(int i=0;i<4;++i){
  const Vector3 r_body{corner_x[i],corner_y[i],half_z};
  const Vector3 r_world=rotate(q,r_body);
  const Vector3 point=add(state_.pose.position_m,r_world);
  const double distance=dot(sub(point,in.terrain.point_m),in.terrain.normal);
  const Vector3 point_velocity=add(state_.linear_velocity_world_mps,
       rotate(q,cross(state_.angular_velocity_body_radps,r_body)));
  const double normal_speed=dot(point_velocity,in.terrain.normal);
  const double load=std::max(0.0,-distance*config_.suspension_stiffness_n_per_m-
                                   normal_speed*config_.suspension_damping_n_s_per_m);
  if(load>0){++out.contact_points;force=add(force,mul(in.terrain.normal,load));
   if(corner_y[i]<0)normal_right+=load;else normal_left+=load;
   torque=add(torque,cross(r_body,inverse_rotate(q,mul(in.terrain.normal,load))));
  }
 }
 out.left_normal_load_n=normal_left;out.right_normal_load_n=normal_right;
 const double motor_tau_left=in.motors.left*config_.maximum_motor_torque_nm*in.battery_voltage_ratio;
 const double motor_tau_right=in.motors.right*config_.maximum_motor_torque_nm*in.battery_voltage_ratio;
 const Vector3 forward=rotate(q,{1,0,0}),side=rotate(q,{0,1,0});
 const auto wheel_force=[&](double omega,double motor_tau,double normal,Vector3 rbody,double& tire_force,double& wheel_accel){
  const Vector3 rw=rotate(q,rbody);
  const Vector3 vel=add(state_.linear_velocity_world_mps,rotate(q,cross(state_.angular_velocity_body_radps,rbody)));
  const double longitudinal=dot(vel,forward);
  const double lateral=dot(vel,side);
  const double slip=omega*config_.wheel_radius_m-longitudinal;
  const double fx=std::clamp(config_.tire_longitudinal_stiffness_n_per_mps*slip,
                             -config_.friction_coefficient*normal,config_.friction_coefficient*normal);
  const double fy=std::clamp(-config_.tire_lateral_stiffness_n_per_mps*lateral,
                             -config_.friction_coefficient*normal,config_.friction_coefficient*normal);
  const double combined=std::hypot(fx,fy),limit=config_.friction_coefficient*normal;
  const double scale=combined>limit&&combined>eps?limit/combined:1.0;
  tire_force=fx*scale;
  const Vector3 f=add(mul(forward,fx*scale),mul(side,fy*scale));
  force=add(force,f);torque=add(torque,cross(rbody,inverse_rotate(q,f)));
  const double resist=config_.rolling_resistance*normal*std::tanh(longitudinal*10.0);
  force=add(force,mul(forward,-resist));
  wheel_accel=(motor_tau-tire_force*config_.wheel_radius_m)/config_.wheel_inertia_kg_m2;
  (void)rw;return combined/(limit>eps?limit:1.0);
 };
 double left_tire=0,right_tire=0,left_acc=0,right_acc=0;
 const double util_l=wheel_force(state_.left_wheel_radps,motor_tau_left,normal_left,{0,half_y,-config_.wheel_radius_m},left_tire,left_acc);
 const double util_r=wheel_force(state_.right_wheel_radps,motor_tau_right,normal_right,{0,-half_y,-config_.wheel_radius_m},right_tire,right_acc);
 const double speed=norm(state_.linear_velocity_world_mps);
 if(speed>eps)force=add(force,mul(state_.linear_velocity_world_mps,-config_.aerodynamic_drag_n_per_mps2*speed));
 out.total_force_world_n=force;out.total_torque_body_nm=torque;
 out.left_tire_force_n=left_tire;out.right_tire_force_n=right_tire;
 out.traction_utilization=std::max(util_l,util_r);
 out.airborne=out.contact_points==0;
 RigidBody3DState next=state_;
 next.linear_velocity_world_mps=add(state_.linear_velocity_world_mps,mul(force,dt/config_.mass_kg));
 next.pose.position_m=add(state_.pose.position_m,mul(next.linear_velocity_world_mps,dt));
 const Vector3 alpha{torque.x/config_.inertia_kg_m2.x,torque.y/config_.inertia_kg_m2.y,torque.z/config_.inertia_kg_m2.z};
 next.angular_velocity_body_radps=add(state_.angular_velocity_body_radps,mul(alpha,dt));
 next.pose.orientation=integrate(q,next.angular_velocity_body_radps,dt);
 const double motor_alpha=config_.motor_time_constant_s==0?1.0:dt/(config_.motor_time_constant_s+dt);
 next.left_wheel_radps+=left_acc*dt;next.right_wheel_radps+=right_acc*dt;
 next.left_wheel_radps+=(motor_tau_left/config_.maximum_motor_torque_nm*40.0-next.left_wheel_radps)*motor_alpha*0.0;
 next.time_s+=dt;
 if(!finite(next.pose.position_m)||!finite(next.linear_velocity_world_mps)||!finite(next.angular_velocity_body_radps))return out;
 state_=next;out.state=state_;out.valid=true;return out;
}
} // namespace oblivion
