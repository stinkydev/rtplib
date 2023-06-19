#include <rtplib/rtp-stream.h>
#include <rtplib/rtp-header.h>
#include "h264.h"
#include <rtplib/rtcp-packets.h>

static const int MAX_MCU = 1400;

RtpStream::RtpStream(const RtpStreamConfig &config, uint32_t packet_sequence_number) {
  socket = std::make_shared<RtpSocket>(config.dst_address, config.src_port, config.dst_port);
    socket->on_receive = [&](std::vector<uint8_t> data) {
    if (RTCP::RTCPPackets::is_rtcp_packet(data.data(), data.size())) {
      rtcp->decode_rtcp(data.data(), data.size());
    }
  };

  this->rtcp_mux = config.rtcp_mux;
  if (rtcp_mux) {
   rtcp = std::make_unique<RTCP::RTCPInstance>(std::string("HEHE"), socket, config.ssrc, config.clock_rate);
  } else {
   rtcp = std::make_unique<RTCP::RTCPInstance>(std::string("HEHE"), config.dst_address, config.src_port_rtcp, config.dst_port_rtcp, config.ssrc, config.clock_rate);
  }

  rtcp->on_request_keyframe = [&]() {
    std::cout << "Requesting keyframe" << std::endl;
    if (on_request_keyframe) {
      on_request_keyframe();
    }
  };

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
  const auto header = create_rtp_header(true, payload_type, packet_sequence_number, ts, ssrc);
  const auto header_size = sizeof(RtpHeader);
  
  const auto total_size = size + header_size;
  const auto buffer = socket->get_buffer();
  memcpy(buffer.data, &header, sizeof(RtpHeader));
  const auto payload_ptr = buffer.data + header_size;
  memcpy(payload_ptr, payload, size);

  socket->send({ buffer.data, total_size }, 0);

  if (rtcp) {
    rtcp->update_stats(packet_sequence_number, total_size, ts);
  }

  packet_sequence_number++;
  return true;
}

bool RtpStream::send(const uint8_t* payload, const size_t size, const uint32_t ts) {
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
    const auto header = create_rtp_header(i == parts - 1, payload_type, packet_sequence_number, ts, ssrc);
    const auto header_size = sizeof(RtpHeader);

    const auto buffer = socket->get_buffer();

    size_t payload_size = (i == parts - 1) ? size - offset : MAX_MCU;
    const size_t payload_offset = (i == 0) ? 1 : 2;

    uint8_t* payload_ptr = buffer.data + header_size + payload_offset;
    uint8_t* fu_header_ptr = payload_ptr - payload_offset;
    const auto total_size = header_size + payload_size + payload_offset;

    memcpy(buffer.data, &header, header_size);
    memcpy(payload_ptr, payload + offset, payload_size);

    *(fu_header_ptr) = fu_indicator;

    if (i == 0) {
      *(fu_header_ptr + 1) = fu_headers[0];
    } else if (i == parts - 1) {
      *(fu_header_ptr + 1) = fu_headers[2];
    } else {
      *(fu_header_ptr + 1) = fu_headers[1];
    }

    offset += payload_size;
    socket->send({ buffer.data, total_size }, i);

    if (rtcp) {
      rtcp->update_stats(packet_sequence_number, total_size, ts);
    }

    packet_sequence_number++;
  }

  return true;
}

bool RtpStream::send_nal_unit(const uint8_t* payload, const size_t size, const uint32_t ts) {
  send_packet(payload, size, ts);
  
  return true;
}