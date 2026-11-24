#include "oblivion/protocol/frame_codec.hpp"
#include <limits>
#include <utility>
namespace oblivion::protocol {
namespace {constexpr std::size_t header_size=10;constexpr std::size_t max_payload=512;}
std::uint16_t crc16_ccitt(const std::uint8_t* data,std::size_t size)noexcept{
 std::uint16_t crc=0xffff;
 for(std::size_t i=0;i<size;++i){crc^=static_cast<std::uint16_t>(data[i])<<8U;for(int bit=0;bit<8;++bit)crc=(crc&0x8000U)?static_cast<std::uint16_t>((crc<<1U)^0x1021U):static_cast<std::uint16_t>(crc<<1U);}
 return crc;
}
std::vector<std::uint8_t> encode(const Packet&p){
 if(p.payload.size()>max_payload) return {};
 std::vector<std::uint8_t> out;out.reserve(header_size+p.payload.size()+2);
 out.push_back(kMagic0);out.push_back(kMagic1);out.push_back(kVersion);out.push_back(static_cast<std::uint8_t>(p.type));
 for(unsigned i=0;i<4;++i)out.push_back(static_cast<std::uint8_t>((p.sequence>>(8U*i))&0xffU));
 const auto n=static_cast<std::uint16_t>(p.payload.size());out.push_back(static_cast<std::uint8_t>(n&0xffU));out.push_back(static_cast<std::uint8_t>(n>>8U));
 out.insert(out.end(),p.payload.begin(),p.payload.end());
 const auto crc=crc16_ccitt(out.data(),out.size());out.push_back(static_cast<std::uint8_t>(crc&0xffU));out.push_back(static_cast<std::uint8_t>(crc>>8U));
 return out;
}
bool decode(const std::uint8_t*b,std::size_t size,Packet&p)noexcept{
 if(b==nullptr||size<header_size+2||b[0]!=kMagic0||b[1]!=kMagic1||b[2]!=kVersion)return false;
 const auto n=static_cast<std::size_t>(b[8])|(static_cast<std::size_t>(b[9])<<8U);
 if(n>max_payload||size!=header_size+n+2)return false;
 const auto expected=static_cast<std::uint16_t>(b[size-2])|static_cast<std::uint16_t>(b[size-1]<<8U);
 if(crc16_ccitt(b,size-2)!=expected)return false;
 if(b[3]<static_cast<std::uint8_t>(MessageType::SensorFrame)||b[3]>static_cast<std::uint8_t>(MessageType::Telemetry))return false;
 Packet candidate{};candidate.type=static_cast<MessageType>(b[3]);
 for(unsigned i=0;i<4;++i)candidate.sequence|=static_cast<std::uint32_t>(b[4+i])<<(8U*i);
 candidate.payload.assign(b+header_size,b+header_size+n);p=std::move(candidate);return true;
}
} // namespace oblivion::protocol
