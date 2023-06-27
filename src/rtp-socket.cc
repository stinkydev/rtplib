#include <rtplib/rtp-socket.h>
#include <iostream>

#define RTP_PACKET_SIZE 1500
#define RTP_POOL_SIZE 100

RtpSocket::RtpSocket(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port) : remote_address(dst_address, dst_port) {
  socket = MinimalSocket::udp::UdpBinded(src_port, remote_address.getFamily());
  socket.open();

  send_buffer = std::make_unique<std::deque<RtpBuffer>>();

  for (size_t i = 0; i < RTP_POOL_SIZE; i++) {
    PoolItem item = PoolItem();
    item.data = new uint8_t[RTP_PACKET_SIZE];
    item.size = RTP_PACKET_SIZE;
    pool.push_back(item);
  }

  thd = std::thread(&RtpSocket::loop, this);
}

RtpSocket::~RtpSocket() {
  stopping = true;
  thd.join();

  for (size_t i = 0; i < pool.size(); i++) {
    delete pool[i].data;
  }
}

void RtpSocket::send(RtpBuffer buffer, uint32_t id) {
  std::lock_guard<std::mutex> lock(mutex);
  send_buffer->push_back(std::move(buffer));
}

RtpBuffer RtpSocket::get_buffer() {
  std::lock_guard<std::mutex> lock(mutex);
  if (pool.size() == 0) {
    PoolItem item = PoolItem();
    item.data = new uint8_t[RTP_PACKET_SIZE];
    item.size = RTP_PACKET_SIZE;
    pool.push_back(item);
  }
  PoolItem item = pool.back();
  pool.pop_back();
  return {item.data, item.size};
}

void RtpSocket::loop() {
  while (!stopping) {
    {
      std::lock_guard<std::mutex> lock(mutex);

      while (send_buffer->size() > 0) {
        RtpBuffer buffer = send_buffer->front();
        send_buffer->pop_front();
        MinimalSocket::ConstBuffer packet{reinterpret_cast<char*>(buffer.data), buffer.size};
        try {
          if (!send_error) {
            socket.sendTo(packet, remote_address);
          }
        } catch (...) {
          // Silent
        }
        pool.push_back({buffer.data, RTP_PACKET_SIZE});
      }
    }
    try {
      if (!receive_error) {
        const auto data = socket.receive(1500, MinimalSocket::Timeout(5));
        if (data.has_value()) {
          const auto packet = data.value();
          if (on_receive) {
            on_receive(std::vector<uint8_t>(packet.received_message.begin(), packet.received_message.end()));
          }
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    } catch (MinimalSocket::SocketError &err) {
      // Silent
      receive_error = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

  }
}
