#include "rtp-stream.h"
#include "rtp-header.h"
#include "h264.h"
#include <MinimalSocket/core/Address.h>
#include <MinimalSocket/udp/UdpSocket.h>

RtpStream::RtpStream(const std::string& dst_address, const uint16_t src_port, const uint16_t dst_port, const uint32_t ssrc, const uint8_t payload_type) :
remote_address(dst_address, dst_port) {
  this->src_port = src_port;
  this->dst_port = dst_port;
  this->ssrc = ssrc;
  this->payload_type = payload_type;
  this->packet_sequence_number = 0;
  
  socket = MinimalSocket::udp::UdpBinded(src_port, remote_address.getFamily());
}

bool RtpStream::send(const uint8_t* payload, const size_t size) {
  const auto offsets = h264::get_nal_offsets(payload, size);
  for (size_t i = 0; i < offsets.size(); i++) {
    const size_t offset = offsets[i];
    const size_t next_offset = (i + 1 < offsets.size()) ? offsets[i + 1] : size;
    const size_t nal_size = next_offset - offset;
    const uint8_t* nal = payload + offset;
    send_nal_unit(nal, nal_size);
  }
  return true;
}

bool RtpStream::send_nal_unit(const uint8_t* payload, const size_t size) {
  RtpHeader header;
  header.set_version(2);
  header.set_padding(0);
  header.set_extension(0);
  header.set_csrc_count(0);
  header.set_marker(0);
  header.set_payload_type(payload_type);
  header.set_sequence_number(packet_sequence_number);
  header.set_timestamp(0);
  header.set_ssrc(ssrc);

  std::vector<uint8_t> header_bytes = header.get_bytes();
  std::vector<uint8_t> packet_bytes;
  packet_bytes.insert(packet_bytes.end(), header_bytes.begin(), header_bytes.end());
  packet_bytes.resize(packet_bytes.size() + size);
  memcpy(packet_bytes.data() + header_bytes.size(), payload, size);

  MinimalSocket::ConstBuffer packet_buffer{reinterpret_cast<char*>(packet_bytes.data()), packet_bytes.size()};
  socket.sendTo(packet_buffer, remote_address);
  packet_sequence_number++;
  
  return true;
}