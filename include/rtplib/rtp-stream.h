#pragma once

#include <stdint.h>
#include <string>
#include <MinimalSocket/udp/UdpSocket.h>
#include <rtplib/rtp-header.h>
#include <rtplib/rtcp.h>

class RtpHeader;

struct RtpStreamConfig {
  std::string dst_address;
  uint16_t dst_port;
  uint16_t src_port;
  uint32_t ssrc;
  uint8_t payload_type;
  uint32_t clock_rate;
};

class RtpStream {
 public:
  RtpStream(const RtpStreamConfig &config);
  bool send_h264(const uint8_t* payload, const size_t size, const uint32_t ts);
  bool send(const uint8_t* payload, const size_t size, const uint32_t ts);
 private:
  std::unique_ptr<RTCP::RTCPInstance> rtcp; 
  std::unique_ptr<RtpHeader> header;
  bool send_big_nal(const uint8_t* payload, const size_t size, const uint32_t ts); 
  bool send_nal_fragment(const uint8_t* payload, const size_t size, const uint32_t ts); 
  bool send_nal_unit(const uint8_t* payload, const size_t size, const uint32_t ts); 
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t ssrc;
  uint8_t payload_type;
  uint32_t packet_sequence_number;
  MinimalSocket::udp::UdpBinded socket;
  MinimalSocket::Address remote_address;
};
