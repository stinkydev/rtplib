#include <rtplib/rtp-stream.h>
#include <rtplib/rtp-header.h>
#include "h264.h"

static const int MAX_MCU = 1400;

RtpStream::RtpStream(const RtpStreamConfig &config, uint32_t packet_sequence_number)
 : socket(config.dst_address, config.src_port, config.dst_port) {
  this->rtcp_mux = config.rtcp_mux;
  if (rtcp_mux) {
   rtcp = std::make_unique<RTCP::RTCPInstance>(std::string("HEHE"), &socket, config.ssrc, config.clock_rate);
  } else {
   rtcp = std::make_unique<RTCP::RTCPInstance>(std::string("HEHE"), config.dst_address, config.src_port_rtcp, config.dst_port_rtcp, config.ssrc, config.clock_rate);
  }
  header = std::make_unique<RtpHeader>();
  this->src_port = config.src_port;
  this->dst_port = config.dst_port;
  this->ssrc = config.ssrc;
  this->payload_type = config.payload_type;
  this->packet_sequence_number = packet_sequence_number;
}

uint32_t RtpStream::get_packet_sequence_number() {
  return packet_sequence_number;
}

bool RtpStream::send_packet(const uint8_t* payload, const size_t size, const uint32_t ts) {
  std::vector<uint8_t> header_bytes = header->get_bytes();
  
  const auto total_size = header_bytes.size() + size;
  const auto buffer = socket.get_buffer();
  memcpy(buffer.data, header_bytes.data(), header_bytes.size());
  const auto payload_ptr = buffer.data + header_bytes.size();
  memcpy(payload_ptr, payload, size);

  socket.send({ buffer.data, total_size }, 0);

  if (rtcp) {
    rtcp->update_stats(packet_sequence_number, total_size, ts);
  }

  packet_sequence_number++;
  return true;
}

bool RtpStream::send(const uint8_t* payload, const size_t size, const uint32_t ts) {
  header->set_version(2);
  header->set_padding(0);
  header->set_extension(0);
  header->set_csrc_count(0);
  header->set_marker(1);
  header->set_payload_type(payload_type);
  header->set_sequence_number(packet_sequence_number);
  header->set_timestamp(ts);
  header->set_ssrc(ssrc);

  send_packet(payload, size, ts);

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
  for (size_t i = 0; i < parts; i++) {
    header->set_version(2);
    header->set_padding(0);
    header->set_extension(0);
    header->set_csrc_count(0);
    header->set_marker(i == parts - 1 ? 1 : 0);
    header->set_payload_type(payload_type);
    header->set_sequence_number(packet_sequence_number);
    header->set_timestamp(ts);
    header->set_ssrc(ssrc);

    std::vector<uint8_t> header_bytes = header->get_bytes();
    const auto buffer = socket.get_buffer();

    size_t payload_size = (i == parts - 1) ? size - offset : MAX_MCU;
    const size_t payload_offset = (i == 0) ? 1 : 2;

    memcpy(buffer.data, header_bytes.data(), header_bytes.size());
    memcpy(buffer.data + header_bytes.size() + payload_offset, payload + offset, payload_size  +payload_offset);
    buffer.data[header_bytes.size()] = fu_indicator;

    if (i == 0) {
      buffer.data[header_bytes.size() + 1] = fu_headers[0];
    } else if (i == parts - 1) {
      buffer.data[header_bytes.size() + 1] = fu_headers[2];
    } else {
      buffer.data[header_bytes.size() + 1] = fu_headers[1];
    }

    offset += payload_size;
    socket.send({ buffer.data, header_bytes.size() + payload_size + payload_offset }, i);

    if (rtcp) {
      rtcp->update_stats(packet_sequence_number, header_bytes.size() + payload_size + payload_offset, ts);
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
  header->set_marker(1);
  header->set_payload_type(payload_type);
  header->set_sequence_number(packet_sequence_number);
  header->set_timestamp(ts);
  header->set_ssrc(ssrc);

  send_packet(payload, size, ts);
  
  return true;
}