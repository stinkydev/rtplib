#pragma once
#include <stdint.h>
#include <string>
#include <mutex>

#include <MinimalSocket/core/Address.h>
#include <MinimalSocket/udp/UdpSocket.h>

#include "rtcp-types.h"
#include <rtplib/rtp-socket.h>

namespace RTCP {
struct SenderStats {
    uint32_t sent_pkts = 0;
    uint32_t sent_bytes = 0;
    bool sent_rtp_packet = false;
};

class RTCPInstance {
 public:
  RTCPInstance(std::string cname, RtpSocket* socket, uint32_t ssrc, uint32_t clock_rate);
  RTCPInstance(std::string cname, const std::string &dst_address, uint16_t src_port, uint16_t dst_port, uint32_t ssrc, uint32_t clock_rate);
  ~RTCPInstance() {
    stopping = true;
    if (rtcp_loop_thread.joinable()) {
      rtcp_loop_thread.join();
    }
  }
  void update_stats(uint32_t rtp_timestamp, uint32_t sent_bytes, uint32_t rtp_ts);
  void stop();
 private:
  void init(const std::string cname);
  bool stopping;
  std::thread rtcp_loop_thread;
  std::mutex stats_mutex_;
  SenderStats our_stats;
  uint32_t last_rtp_timestamp;
  uint64_t last_ntp_timestamp;
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t ssrc;
  RtpSocket* socket;
  std::mutex packet_mutex_;
  uint32_t rtcp_pkt_sent_count_;
  char cname_[255];
  rtcp_sdes_item cnameItem_;
  uint64_t clock_start_;
  uint32_t clock_rate_;
  std::vector<rtcp_sdes_item> ourItems_;
  bool generate_report();
  void rtcp_loop();

  uint32_t size_of_compound_packet(uint16_t reports, bool sr_packet, bool sdes_packet, bool bye_packet) const;
};

}