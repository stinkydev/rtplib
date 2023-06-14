#include <rtplib/rtp-stream.h>
#include <rtplib/rtp-header.h>
#include "h264.h"
#include <MinimalSocket/core/Address.h>
#include <MinimalSocket/udp/UdpSocket.h>

static const int MAX_MCU = 1400;

RtpStream::RtpStream(const RtpStreamConfig &config) : remote_address(config.dst_address, config.dst_port) {
  if (true) {
    rtcp = std::make_unique<RTCP::RTCPInstance>(std::string("HEHE"), config.dst_address, config.src_port_rtcp, config.dst_port_rtcp, config.ssrc, config.clock_rate);
  } else {
    rtcp = nullptr;
  }
  header = std::make_unique<RtpHeader>();
  this->src_port = config.src_port;
  this->dst_port = config.dst_port;
  this->ssrc = config.ssrc;
  this->payload_type = config.payload_type;
  this->packet_sequence_number = 0;
  
  socket = MinimalSocket::udp::UdpBinded(src_port, remote_address.getFamily());
  socket.open();
}

bool RtpStream::send(const uint8_t* payload, const size_t size, const uint32_t ts) {
  if (size > MAX_MCU) {
    return false;
  }
  header->set_version(2);
  header->set_padding(0);
  header->set_extension(0);
  header->set_csrc_count(0);
  header->set_marker(0);
  header->set_payload_type(payload_type);
  header->set_sequence_number(packet_sequence_number);
  header->set_timestamp(ts);
  header->set_ssrc(ssrc);

  std::vector<uint8_t> header_bytes = header->get_bytes();
  std::vector<uint8_t> packet_bytes;
  packet_sequence_number++;
  packet_bytes.insert(packet_bytes.end(), header_bytes.begin(), header_bytes.end());
  packet_bytes.resize(header_bytes.size() + size);
  memcpy(packet_bytes.data() + header_bytes.size(), payload, size);
  MinimalSocket::ConstBuffer packet_buffer{reinterpret_cast<char*>(packet_bytes.data()), packet_bytes.size()};
  // std::cout << "sending audio " << packet_bytes.size() << " to " << remote_address.getHost() << std::endl;
  socket.sendTo(packet_buffer, remote_address);

  if (rtcp) {
    rtcp->update_stats(packet_sequence_number, packet_bytes.size(), ts);
  }

  packet_sequence_number++;
  return true; 
}


bool RtpStream::send_h264(const uint8_t* payload, const size_t size, const uint32_t ts) {
  const auto offsets = h264::get_nal_offsets(payload, size);
  for (size_t i = 0; i < offsets.size(); i++) {
    const size_t offset = offsets[i];
    const size_t next_offset = (i + 1 < offsets.size()) ? offsets[i + 1] - 3 : size;
    size_t nal_size = next_offset - offset;
    const uint8_t* nal = payload + offset;
    if (nal[nal_size - 1] == 0) {
      nal_size--;
    }

    if (nal_size <= MAX_MCU) {
      send_nal_unit(nal, nal_size, ts);
    } else {
      send_big_nal(nal, nal_size, ts);
    }

  }
  return true;
}

bool RtpStream::send_nal_fragment(const uint8_t* payload, const size_t size, const uint32_t ts) {
  return true;
}

bool RtpStream::send_big_nal(const uint8_t* payload, const size_t size, const uint32_t ts) {
  const int parts = std::ceil((double)size / MAX_MCU);

  uint8_t nal_type = payload[0] & 0x1f;
  uint8_t nal_header = payload[0];
  uint8_t fu_indicator = (nal_header & 0x60) | 28;

  uint8_t fu_headers[3];
  fu_headers[0] = (uint8_t)((1 << 7) | nal_type);
  fu_headers[1] = nal_type;
  fu_headers[2] = (uint8_t)((1 << 6) | nal_type);

  size_t offset = 0;
  //std::cout << "size: " << size << " " << parts << " parts" << std::endl;
  for (size_t i = 0; i < parts; i++) {
    header->set_version(2);
    header->set_padding(0);
    header->set_extension(0);
    header->set_csrc_count(0);
    header->set_marker(0);
    header->set_payload_type(payload_type);
    header->set_sequence_number(packet_sequence_number);
    header->set_timestamp(ts);
    header->set_ssrc(ssrc);

    std::vector<uint8_t> header_bytes = header->get_bytes();
    std::vector<uint8_t> packet_bytes;

    size_t payload_size = (i == parts - 1) ? size - offset : MAX_MCU;
    const size_t payload_offset = (i == 0) ? 1 : 2;

    packet_bytes.insert(packet_bytes.end(), header_bytes.begin(), header_bytes.end());
    packet_bytes.resize(header_bytes.size() + payload_size + payload_offset);
    memcpy(packet_bytes.data() + header_bytes.size() + payload_offset, payload + offset, payload_size);
    packet_bytes[header_bytes.size()] = fu_indicator;

    if (i == 0) {
      packet_bytes[header_bytes.size() + 1] = fu_headers[0];
    } else if (i == parts - 1) {
      packet_bytes[header_bytes.size() + 1] = fu_headers[2];
    } else {
      packet_bytes[header_bytes.size() + 1] = fu_headers[1];
    }

    offset += payload_size;
    MinimalSocket::ConstBuffer packet_buffer{reinterpret_cast<char*>(packet_bytes.data()), packet_bytes.size()};
    //std::cout << "BIG sending " << packet_bytes.size() << " to " << remote_address.getHost() << std::endl;
    socket.sendTo(packet_buffer, remote_address);

    if (rtcp) {
      rtcp->update_stats(packet_sequence_number, packet_bytes.size(), ts);
    }

    packet_sequence_number++;
  }

  return true;
}

bool RtpStream::send_nal_unit(const uint8_t* payload, const size_t size, const uint32_t ts) {
  header->set_version(2);
  header->set_padding(0);
  header->set_extension(0);
  header->set_csrc_count(0);
  header->set_marker(0);
  header->set_payload_type(payload_type);
  header->set_sequence_number(packet_sequence_number);
  header->set_timestamp(ts);
  header->set_ssrc(ssrc);

  std::vector<uint8_t> header_bytes = header->get_bytes();
  std::vector<uint8_t> packet_bytes;
  packet_bytes.insert(packet_bytes.end(), header_bytes.begin(), header_bytes.end());
  packet_bytes.resize(packet_bytes.size() + size);
  memcpy(packet_bytes.data() + header_bytes.size(), payload, size);

  MinimalSocket::ConstBuffer packet_buffer{reinterpret_cast<char*>(packet_bytes.data()), packet_bytes.size()};
  //std::cout << "sending " << packet_bytes.size() << " to " << remote_address.getHost() << std::endl;
  socket.sendTo(packet_buffer, remote_address);

  if (rtcp) {
    rtcp->update_stats(packet_sequence_number, packet_bytes.size(), ts);
  }

  packet_sequence_number++;
  
  return true;
}