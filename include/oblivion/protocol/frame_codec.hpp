#pragma once
#include <cstdint>
#include <vector>
namespace oblivion::protocol {
constexpr std::uint8_t kVersion=1;
constexpr std::uint8_t kMagic0=0x4f,kMagic1=0x42;
enum class MessageType:std::uint8_t{SensorFrame=1,MotorCommand=2,Heartbeat=3,EmergencyStop=4,Calibration=5};
struct Packet{MessageType type{MessageType::Heartbeat};std::uint32_t sequence{0};std::vector<std::uint8_t> payload;};
std::uint16_t crc16_ccitt(const std::uint8_t* data,std::size_t size)noexcept;
std::vector<std::uint8_t> encode(const Packet& packet);
bool decode(const std::uint8_t* bytes,std::size_t size,Packet& packet)noexcept;
} // namespace oblivion::protocol
