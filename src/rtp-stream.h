#pragma once

#include <stdint.h>
#include <string>
#include <MinimalSocket/udp/UdpSocket.h>

class RtpStream {
 public:
  RtpStream(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port, const uint32_t ssrc, const uint8_t payload_type);
  bool send(const uint8_t* payload, const size_t size);
 private:
  bool send_nal_unit(const uint8_t* payload, const size_t size); 
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t ssrc;
  uint8_t payload_type;
  uint32_t packet_sequence_number;
  MinimalSocket::udp::UdpBinded socket;
  MinimalSocket::Address remote_address;
};
