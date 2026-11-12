#include "oblivion/simulation/track_simulator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace oblivion {
namespace {
double dist(TrackPoint a,TrackPoint b)noexcept{return std::hypot(a.x_m-b.x_m,a.y_m-b.y_m);}
double segment_distance(TrackPoint p,TrackPoint a,TrackPoint b)noexcept{
 const double dx=b.x_m-a.x_m,dy=b.y_m-a.y_m,l2=dx*dx+dy*dy;
 if(l2<1e-18)return dist(p,a);
 const double u=std::clamp(((p.x_m-a.x_m)*dx+(p.y_m-a.y_m)*dy)/l2,0.0,1.0);
 return dist(p,{a.x_m+u*dx,a.y_m+u*dy});
}
}
bool Track::valid()const noexcept{
 if(point_count<2||point_count>centerline.size()||branch_count>branches.size()||
    !std::isfinite(line_width_m)||line_width_m<=0)return false;
 for(std::size_t i=0;i<point_count;++i)
  if(!std::isfinite(centerline[i].x_m)||!std::isfinite(centerline[i].y_m))return false;
 for(std::size_t b=0;b<branch_count;++b){
  if(branches[b].point_count<2||branches[b].point_count>branches[b].points.size())return false;
  for(std::size_t i=0;i<branches[b].point_count;++i)
   if(!std::isfinite(branches[b].points[i].x_m)||!std::isfinite(branches[b].points[i].y_m))return false;
 }
 return true;
}
double Track::distance_to_centerline(TrackPoint p)const noexcept{
 if(!valid()||!std::isfinite(p.x_m)||!std::isfinite(p.y_m))return std::numeric_limits<double>::infinity();
 double best=std::numeric_limits<double>::infinity();
 for(std::size_t i=1;i<point_count;++i)best=std::min(best,segment_distance(p,centerline[i-1],centerline[i]));
 return best;
}
double Track::distance_to_visible_line(TrackPoint p)const noexcept{
 if(!valid()||!std::isfinite(p.x_m)||!std::isfinite(p.y_m))return std::numeric_limits<double>::infinity();
 double best=std::numeric_limits<double>::infinity();
 for(std::size_t i=1;i<point_count;++i)if(visible_segments[i])
  best=std::min(best,segment_distance(p,centerline[i-1],centerline[i]));
 for(std::size_t b=0;b<branch_count;++b)
  for(std::size_t i=1;i<branches[b].point_count;++i)
   best=std::min(best,segment_distance(p,branches[b].points[i-1],branches[b].points[i]));
 return best;
}
TrackBuilder::TrackBuilder(double w)noexcept{track_.line_width_m=w;}
bool TrackBuilder::append(TrackPoint p)noexcept{
 if(track_.point_count>=track_.centerline.size()||!std::isfinite(p.x_m)||!std::isfinite(p.y_m))return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],p)<1e-9)return true;
 const std::size_t index=track_.point_count++;
 track_.centerline[index]=p;
 if(index>0)track_.visible_segments[index]=true;
 return true;
}
bool TrackBuilder::add_straight(TrackPoint a,TrackPoint b)noexcept{
 if(!std::isfinite(a.x_m)||!std::isfinite(a.y_m)||!std::isfinite(b.x_m)||!std::isfinite(b.y_m)||dist(a,b)<1e-6)return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],a)>1e-6)return false;
 return append(a)&&append(b);
}
bool TrackBuilder::add_polyline(const TrackPoint*p,std::size_t n)noexcept{
 if(!p||n<2||n>kMaxTrackPoints-track_.point_count+(track_.point_count?1U:0U))return false;
 for(std::size_t i=0;i<n;++i)if(!std::isfinite(p[i].x_m)||!std::isfinite(p[i].y_m))return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],p[0])>1e-6)return false;
 for(std::size_t i=0;i<n;++i)if(!append(p[i]))return false;
 return true;
}
bool TrackBuilder::add_arc(TrackPoint c,double r,double start,double sweep,std::size_t n)noexcept{
 if(!std::isfinite(c.x_m)||!std::isfinite(c.y_m)||!std::isfinite(r)||!std::isfinite(start)||!std::isfinite(sweep)||r<=0||std::abs(sweep)<1e-6||n<2||n>kMaxTrackPoints)return false;
 if(track_.point_count==0 ? n>=kMaxTrackPoints : n>kMaxTrackPoints-track_.point_count)return false;
 const TrackPoint first{c.x_m+r*std::cos(start),c.y_m+r*std::sin(start)};
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],first)>1e-5)return false;
 for(std::size_t i=0;i<=n;++i){const double a=start+sweep*static_cast<double>(i)/static_cast<double>(n);if(!append({c.x_m+r*std::cos(a),c.y_m+r*std::sin(a)}))return false;}
 return true;
}
bool TrackBuilder::add_s_bend(TrackPoint start,double length,double amplitude,std::size_t n)noexcept{
 if(!std::isfinite(length)||!std::isfinite(amplitude)||length<=0||std::abs(amplitude)<1e-5||n<4||n>kMaxTrackPoints)return false;
 if(n>kMaxTrackPoints-track_.point_count+1)return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],start)>1e-6)return false;
 for(std::size_t i=0;i<=n;++i){const double u=static_cast<double>(i)/static_cast<double>(n);
  if(!append({start.x_m+length*u,start.y_m+amplitude*std::sin(2.0*3.14159265358979323846*u)}))return false;}
 return true;
}
bool TrackBuilder::add_hairpin(TrackPoint start,double radius,int direction,std::size_t n)noexcept{
 if(!std::isfinite(radius)||radius<=0||(direction!=1&&direction!=-1)||n<4||n>kMaxTrackPoints)return false;
 if(n>kMaxTrackPoints-track_.point_count+1)return false;
 if(track_.point_count&&dist(track_.centerline[track_.point_count-1],start)>1e-6)return false;
 const double sign=static_cast<double>(direction);
 const TrackPoint center{start.x_m,start.y_m+sign*radius};
 return add_arc(center,radius,-sign*1.57079632679489661923,sign*3.14159265358979323846,n);
}
bool TrackBuilder::add_gap(double start_distance,double length)noexcept{
 if(track_.point_count<2||!std::isfinite(start_distance)||!std::isfinite(length)||start_distance<0||length<=0)return false;
 double traversed=0.0;bool changed=false;
 for(std::size_t i=1;i<track_.point_count;++i){
  const double segment=dist(track_.centerline[i-1],track_.centerline[i]);
  const double end=traversed+segment;
  if(end>start_distance&&traversed<start_distance+length){track_.visible_segments[i]=false;changed=true;}
  traversed=end;
 }
 return changed;
}
bool TrackBuilder::add_branch(TrackBranchKind kind,const TrackPoint*p,std::size_t n)noexcept{
 if(!p||n<2||n>kMaxBranchPoints||track_.branch_count>=track_.branches.size())return false;
 for(std::size_t i=0;i<n;++i)if(!std::isfinite(p[i].x_m)||!std::isfinite(p[i].y_m))return false;
 auto&branch=track_.branches[track_.branch_count];branch.kind=kind;branch.point_count=n;
 for(std::size_t i=0;i<n;++i)branch.points[i]=p[i];
 ++track_.branch_count;return true;
}
Track TrackBuilder::build()const noexcept{return track_;}
ReflectanceArraySimulator::ReflectanceArraySimulator(SimulatedReflectanceConfig c)noexcept:config_(c),random_state_(c.random_seed?c.random_seed:1U){}
bool ReflectanceArraySimulator::configuration_valid()const noexcept{
 const auto&c=config_;if(c.sensor_count<2||c.sensor_count>c.lateral_positions_m.size()||!std::isfinite(c.white_level)||!std::isfinite(c.dark_level)||!std::isfinite(c.noise_amplitude)||!std::isfinite(c.dropout_probability)||c.white_level<=c.dark_level||c.noise_amplitude<0||c.noise_amplitude>(c.white_level-c.dark_level)*0.5||c.dropout_probability<0||c.dropout_probability>=1)return false;
 for(std::size_t i=0;i<c.sensor_count;++i)if(!std::isfinite(c.lateral_positions_m[i])||(i&&c.lateral_positions_m[i]<=c.lateral_positions_m[i-1]))return false;
 return true;
}
double ReflectanceArraySimulator::random_unit()noexcept{
 random_state_^=random_state_<<13U;random_state_^=random_state_>>17U;random_state_^=random_state_<<5U;
 return static_cast<double>(random_state_)/static_cast<double>(0xFFFFFFFFU);
}
double ReflectanceArraySimulator::noise()noexcept{return (2.0*random_unit()-1.0)*config_.noise_amplitude;}
bool ReflectanceArraySimulator::sample(const Track&t,const Pose2D&p,std::uint64_t timestamp,double*out,std::size_t cap)noexcept{
 (void)timestamp;if(!configuration_valid()||!t.valid()||!out||cap<config_.sensor_count||!std::isfinite(p.x_m)||!std::isfinite(p.y_m)||!std::isfinite(p.heading_rad))return false;
 const double nx=-std::sin(p.heading_rad),ny=std::cos(p.heading_rad);
 for(std::size_t i=0;i<config_.sensor_count;++i){
  if(random_unit()<config_.dropout_probability){out[i]=std::numeric_limits<double>::quiet_NaN();continue;}
  const TrackPoint sensor{p.x_m+nx*config_.lateral_positions_m[i],p.y_m+ny*config_.lateral_positions_m[i]};
  const bool line=t.distance_to_visible_line(sensor)<=0.5*t.line_width_m;
  out[i]=std::clamp((line?config_.dark_level:config_.white_level)+noise(),config_.dark_level,config_.white_level);
 }
 return true;
}
} // namespace oblivion
