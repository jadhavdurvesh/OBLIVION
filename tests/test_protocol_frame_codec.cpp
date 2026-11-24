#include "oblivion/protocol/frame_codec.hpp"
#include <stdexcept>
void test_protocol_frame_codec(){
 using namespace oblivion::protocol;
 Packet packet{};packet.type=MessageType::MotorCommand;packet.sequence=0x12345678U;packet.payload={0x01,0x02,0x03,0xfe};
 const auto bytes=encode(packet);Packet decoded{};
 if(bytes.empty()||!decode(bytes.data(),bytes.size(),decoded))throw std::runtime_error("valid protocol frame did not decode");
 if(decoded.type!=packet.type||decoded.sequence!=packet.sequence||decoded.payload!=packet.payload)throw std::runtime_error("protocol frame round trip mismatch");
 auto damaged=bytes;damaged[11]^=0x40;
 if(decode(damaged.data(),damaged.size(),decoded))throw std::runtime_error("protocol CRC did not reject corrupted payload");
 if(decode(bytes.data(),bytes.size()-1,decoded))throw std::runtime_error("protocol decoder accepted truncated frame");
}
