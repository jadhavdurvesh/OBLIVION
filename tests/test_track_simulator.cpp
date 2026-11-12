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
}
