#include "oblivion/simulation/rigid_body_3d.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
int main(int argc,char**argv){
 using namespace oblivion;
 const std::string path=argc>1?argv[1]:"oblivion_3d_trace.json";
 RigidBody3DConfig config{};
 RigidBody3DSimulator sim(config);
 if(!sim.configuration_valid())return 1;
 RigidBody3DState initial{};initial.pose.position_m={0.0,0.0219,0.0};
 sim.reset(initial);
 std::ofstream out(path);if(!out){std::cerr<<"Cannot write trace: "<<path<<'\n';return 2;}
 out<<"{\n  \"format\": \"oblivion-3d-trace-v1\",\n  \"units\": \"SI\",\n  \"samples\": [\n";
 constexpr double dt=0.001;constexpr int steps=6000;
 RigidBody3DOutput last_output{};
 for(int i=0;i<=steps;++i){
  const double t=i*dt;
  if(i>0){
   const double steer=0.24*std::sin(2.0*3.14159265358979323846*t/3.2);
   RigidBody3DInput input{};input.motors={0.48-steer,0.48+steer};
   last_output=sim.step(input,dt);if(!last_output.valid){std::cerr<<"3D simulation became invalid at "<<t<<" s\n";return 3;}
  }
  const auto&s=sim.state();
  const auto&q=s.pose.orientation;
  const double roll=std::atan2(2.0*(q.w*q.x+q.y*q.z),1.0-2.0*(q.x*q.x+q.y*q.y));
  const double pitch=std::asin(std::clamp(2.0*(q.w*q.y-q.z*q.x),-1.0,1.0));
  if(i)out<<",\n";
  out<<"    {\"t\":"<<std::setprecision(9)<<s.time_s
     <<",\"x\":"<<s.pose.position_m.x<<",\"y\":"<<s.pose.position_m.y
     <<",\"z\":"<<s.pose.position_m.z<<",\"roll\":"<<roll<<",\"pitch\":"<<pitch
     <<",\"q\":{\"w\":"<<q.w<<",\"x\":"<<q.x<<",\"y\":"<<q.y<<",\"z\":"<<q.z<<"}"
     <<",\"velocity\":{\"x\":"<<s.linear_velocity_world_mps.x
     <<",\"y\":"<<s.linear_velocity_world_mps.y<<",\"z\":"<<s.linear_velocity_world_mps.z<<"}"
     <<",\"contacts\":"<<(i==0?4:last_output.contact_points)<<",\"airborne\":"<<(i>0&&last_output.airborne?"true":"false")<<"}";
 }
 out<<"\n  ]\n}\n";out.close();
 std::cout<<"Wrote "<<steps+1<<" 3D physics samples to "<<path<<'\n';
 return 0;
}
