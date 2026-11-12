#include "oblivion/simulation/track_simulator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace oblivion {
namespace {double dist(TrackPoint a,TrackPoint b)noexcept{return std::hypot(a.x_m-b.x_m,a.y_m-b.y_m);}}
bool Track::valid()const noexcept{
 if(point_count<2||point_count>centerline.size()||!std::isfinite(line_width_m)||line_width_m<=0)return false;
 for(std::size_t i=0;i<point_count;++i)if(!std::isfinite(centerline[i].x_m)||!std::isfinite(centerline[i].y_m))return false;
 return true;
}
double Track::distance_to_centerline(TrackPoint p)const noexcept{
 if(!valid()||!std::isfinite(p.x_m)||!std::isfinite(p.y_m))return std::numeric_limits<double>::infinity();
 double best=std::numeric_limits<double>::infinity();
 for(std::size_t i=1;i<point_count;++i){
  const auto a=centerline[i-1],b=centerline[i];const double dx=b.x_m-a.x_m,dy=b.y_m-a.y_m;
  const double l2=dx*dx+dy*dy;if(l2<1e-18)continue;
  const double t=std::clamp(((p.x_m-a.x_m)*dx+(p.y_m-a.y_m)*dy)/l2,0.0,1.0);
  best=std::min(best,dist(p,{a.x_m+t*dx,a.y_m+t*dy}));
 }
 return best;
}
TrackBuilder::TrackBuilder(double w)noexcept{track_.line_width_m=w;}
bool TrackBuilder::append(TrackPoint p)noexcept{
 if(track_.point_count>=track_.centerline.size()||!std::isfinite(p.x_m)||!std::isfinite(p.y_m))return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],p)<1e-9)return true;
 track_.centerline[track_.point_count++]=p;return true;
}
bool TrackBuilder::add_straight(TrackPoint a,TrackPoint b)noexcept{
 if(!std::isfinite(a.x_m)||!std::isfinite(a.y_m)||!std::isfinite(b.x_m)||!std::isfinite(b.y_m)||dist(a,b)<1e-6)return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],a)>1e-6)return false;
 return append(a)&&append(b);
}
bool TrackBuilder::add_arc(TrackPoint c,double r,double start,double sweep,std::size_t n)noexcept{
 if(!std::isfinite(c.x_m)||!std::isfinite(c.y_m)||!std::isfinite(r)||!std::isfinite(start)||!std::isfinite(sweep)||r<=0||std::abs(sweep)<1e-6||n<2||n>kMaxTrackPoints)return false;
 const TrackPoint first{c.x_m+r*std::cos(start),c.y_m+r*std::sin(start)};
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],first)>1e-5)return false;
 if(!append(first))return false;
 for(std::size_t i=1;i<=n;++i){const double a=start+sweep*static_cast<double>(i)/static_cast<double>(n);if(!append({c.x_m+r*std::cos(a),c.y_m+r*std::sin(a)}))return false;}
 return true;
}
Track TrackBuilder::build()const noexcept{return track_;}
ReflectanceArraySimulator::ReflectanceArraySimulator(SimulatedReflectanceConfig c)noexcept:config_(c),random_state_(c.random_seed?c.random_seed:1U){}
bool ReflectanceArraySimulator::configuration_valid()const noexcept{
 const auto&c=config_;if(c.sensor_count<2||c.sensor_count>c.lateral_positions_m.size()||!std::isfinite(c.white_level)||!std::isfinite(c.dark_level)||!std::isfinite(c.noise_amplitude)||c.white_level<=c.dark_level||c.noise_amplitude<0||c.noise_amplitude>(c.white_level-c.dark_level)*0.5)return false;
 for(std::size_t i=0;i<c.sensor_count;++i)if(!std::isfinite(c.lateral_positions_m[i])||(i&&c.lateral_positions_m[i]<=c.lateral_positions_m[i-1]))return false;
 return true;
}
double ReflectanceArraySimulator::noise()noexcept{
 random_state_^=random_state_<<13U;random_state_^=random_state_>>17U;random_state_^=random_state_<<5U;
 const double unit=static_cast<double>(random_state_)/static_cast<double>(0xFFFFFFFFU);
 return (2.0*unit-1.0)*config_.noise_amplitude;
}
bool ReflectanceArraySimulator::sample(const Track&t,const Pose2D&p,std::uint64_t timestamp,double*out,std::size_t cap)noexcept{
 (void)timestamp;if(!configuration_valid()||!t.valid()||!out||cap<config_.sensor_count||!std::isfinite(p.x_m)||!std::isfinite(p.y_m)||!std::isfinite(p.heading_rad))return false;
 const double nx=-std::sin(p.heading_rad),ny=std::cos(p.heading_rad);
 for(std::size_t i=0;i<config_.sensor_count;++i){
  const TrackPoint sensor{p.x_m+nx*config_.lateral_positions_m[i],p.y_m+ny*config_.lateral_positions_m[i]};
  const bool line=t.distance_to_centerline(sensor)<=0.5*t.line_width_m;
  out[i]=std::clamp((line?config_.dark_level:config_.white_level)+noise(),config_.dark_level,config_.white_level);
 }
 return true;
}
} // namespace oblivion
