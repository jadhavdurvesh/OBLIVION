#include "oblivion/simulation/track_simulator.hpp"
#include <cmath>
#include <stdexcept>
namespace {void require(bool c,const char*m){if(!c)throw std::runtime_error(m);}}
void test_track_simulator(){
 using namespace oblivion;TrackBuilder b(0.012);
 require(b.add_straight({-1,0},{1,0}),"straight track generation failed");
 const Track t=b.build();require(t.valid(),"track invalid");
 require(t.distance_to_centerline({0,0})<1e-9,"centerline distance wrong");
 require(std::abs(t.distance_to_centerline({0,0.1})-0.1)<1e-9,"off-track distance wrong");
 SimulatedReflectanceConfig c{};c.sensor_count=5;
 for(std::size_t i=0;i<5;++i)c.lateral_positions_m[i]=(static_cast<double>(i)-2.0)*0.01;
 ReflectanceArraySimulator sensors(c);double values[5]{};
 require(sensors.sample(t,{0,0,0},1000,values,5),"sensor sampling failed");
 require(values[2]<0.1&&values[0]>0.9&&values[4]>0.9,"sensor readings incorrect");
 require(!sensors.sample(t,{0,0,0},1000,values,4),"undersized output accepted");
 TrackBuilder arc(0.01);require(arc.add_arc({0,0},0.5,0,1.57079632679,16),"arc generation failed");require(arc.build().valid(),"arc track invalid");
 TrackBuilder sbend(0.012);require(sbend.add_s_bend({0,0},0.8,0.12,40),"S-bend generation failed");
 require(sbend.build().valid(),"S-bend track invalid");
 TrackBuilder hairpin(0.012);require(hairpin.add_hairpin({0,0},0.15,1,24),"hairpin generation failed");
 require(hairpin.build().valid(),"hairpin track invalid");
 TrackBuilder broken(0.012);require(broken.add_straight({0,0},{1,0}),"gap base track failed");
 require(broken.add_gap(0.35,0.15),"gap insertion failed");
 const auto broken_track=broken.build();
 require(broken_track.distance_to_centerline({0.4,0})<1e-9,"gap changed track geometry");
 require(broken_track.distance_to_visible_line({0.4,0})>0.05,"gap remains visible to virtual sensor");
 const TrackPoint fork[]={{0.5,0.0},{0.6,0.1},{0.8,0.2}};
 TrackBuilder branched(0.012);require(branched.add_straight({0,0},{1,0}),"branch base track failed");
 require(branched.add_branch(TrackBranchKind::Fork,fork,3),"branch insertion failed");
 require(branched.build().valid(),"branched track invalid");
 require(branched.build().distance_to_visible_line({0.7,0.15})<0.02,"branch not visible to virtual sensor");
 SimulatedReflectanceConfig dropout{};dropout.sensor_count=5;dropout.dropout_probability=0.99;dropout.random_seed=1234;
 for(std::size_t i=0;i<5;++i)dropout.lateral_positions_m[i]=(static_cast<double>(i)-2.0)*0.01;
 ReflectanceArraySimulator failing_sensors(dropout);double failed[5]{};
 require(failing_sensors.sample(t,{0,0,0},2000,failed,5),"dropout sensor sampling failed");
 bool saw_dropout=false;for(double value:failed)saw_dropout=saw_dropout||!std::isfinite(value);
 require(saw_dropout,"sensor dropout fault was not injected");
}
