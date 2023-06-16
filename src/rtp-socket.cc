#include <rtplib/rtp-socket.h>

#define RTP_PACKET_SIZE 1500
#define RTP_POOL_SIZE 100

RtpSocket::RtpSocket(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port) : remote_address(dst_address, dst_port) {
  socket = MinimalSocket::udp::UdpBinded(src_port, remote_address.getFamily());
  socket.open();

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

void RtpSocket::send(RtpBuffer buffer) {
  std::lock_guard<std::mutex> lock(mutex);
  buffers.push_back(buffer);
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
    std::lock_guard<std::mutex> lock(mutex);

    while (buffers.size() > 0) {
      RtpBuffer buffer = buffers.front();
      buffers.pop_front();
      MinimalSocket::ConstBuffer packet{reinterpret_cast<char*>(buffer.data), buffer.size};

      socket.sendTo(packet, remote_address);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
