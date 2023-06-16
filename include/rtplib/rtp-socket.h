#pragma once

#include <stdint.h>
#include <string>
#include <thread>
#include <deque>
#include <vector>

#include <MinimalSocket/core/Address.h>
#include <MinimalSocket/udp/UdpSocket.h>
#include <rtplib/rtp-header.h>

struct RtpBuffer {
  uint8_t* data;
  size_t size;
};

struct PoolItem {
  uint8_t* data;
  size_t size;
};

class RtpSocket {
 private:
  bool stopping = false;
  std::deque<RtpBuffer> buffers;
  MinimalSocket::udp::UdpBinded socket;
  MinimalSocket::Address remote_address;
  std::thread thd;
  std::vector<PoolItem> pool;
  void loop();
  std::mutex mutex;
 public:
  void send(RtpBuffer buffer);
  RtpBuffer get_buffer();
  RtpSocket(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port);
  ~RtpSocket();
};
