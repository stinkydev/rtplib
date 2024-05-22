#pragma once

#include <stdint.h>
#include <string>
#include <rtplib/rtp-header.h>
#include <rtplib/rtcp.h>
#include <rtplib/rtp-socket.h>

struct RtpStreamConfig {
  std::string dst_address;
  uint16_t dst_port;
  uint16_t src_port;
  uint16_t dst_port_rtcp;
  uint16_t src_port_rtcp;
  uint32_t ssrc;
  uint8_t payload_type;
  uint32_t clock_rate;
  bool rtcp_mux;
};

class RtpStream {
 public:
  RtpStream(const RtpStreamConfig &config, uint32_t packet_sequence_number = 0);
  ~RtpStream() {
    if (rtcp) {
      rtcp->stop();
    }
  }
  bool send_h264(const uint8_t* payload, const size_t size, const uint32_t ts);
  bool send(const uint8_t* payload, const size_t size, const uint32_t ts);
  uint32_t get_packet_sequence_number();
  std::function <void()> on_request_keyframe = nullptr;
 private:
  bool send_packet(const uint8_t* payload, const size_t size, const uint32_t ts);
  std::unique_ptr<RTCP::RTCPInstance> rtcp; 
  bool send_big_nal(const uint8_t* payload, const size_t size, const uint32_t ts); 
  bool send_nal_fragment(const uint8_t* payload, const size_t size, const uint32_t ts); 
  bool send_nal_unit(const uint8_t* payload, const size_t size, const uint32_t ts);
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t ssrc;
  uint8_t payload_type;
  uint32_t packet_sequence_number;
  std::shared_ptr<RtpSocket> socket;
  bool rtcp_mux;
};
