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
  std::unique_ptr<std::deque<RtpBuffer>> send_buffer;
  MinimalSocket::udp::UdpBinded socket;
  MinimalSocket::Address remote_address;
  std::thread thd;
  std::vector<PoolItem> pool;
  void loop();
  std::mutex mutex;
 public:
  std::function<void(std::vector<uint8_t>)> on_receive = nullptr;
  void send(RtpBuffer buffer, uint32_t id);
  RtpBuffer get_buffer();
  RtpSocket(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port);
  ~RtpSocket();
};
