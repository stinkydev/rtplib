#include <rtplib/rtcp.h>
#include <rtplib/rtcp-packets.h>
#include <rtplib/clock.h>
#include <iostream>
#include <random>

namespace RTCP {

RTCPInstance::RTCPInstance(std::string cname, RtpSocket* socket, uint32_t ssrc, uint32_t clock_rate) : socket(socket), ssrc(ssrc), clock_rate_(clock_rate) {
  init(cname);
}

RTCPInstance::RTCPInstance(std::string cname, const std::string &dst_address, uint16_t src_port, uint16_t dst_port, uint32_t ssrc, uint32_t clock_rate)
  : src_port(src_port), dst_port(dst_port), ssrc(ssrc), clock_rate_(clock_rate) {
  socket = new RtpSocket(dst_address, src_port, dst_port);
  init(cname);
}

void RTCPInstance::init(std::string cname) {
  rtcp_pkt_sent_count_ = 0;
  clock_start_ = 0;
  last_ntp_timestamp = 0;
  last_rtp_timestamp = 0;

  // items should not have null termination
  const char* c = cname.c_str();
  memcpy(cname_, c, cname.length());
  uint8_t length = (uint8_t)cname.length();

  cnameItem_.type = 1;
  cnameItem_.length = length;
  cnameItem_.data = (uint8_t*)cname_;

  ourItems_.push_back(cnameItem_);

  stopping = false;
  rtcp_loop_thread = std::thread(&RTCPInstance::rtcp_loop, this);
}

void RTCPInstance::stop() {
  stopping = true;
  if (rtcp_loop_thread.joinable()) {
    rtcp_loop_thread.join();
  }
}

void RTCPInstance::rtcp_loop() {
  srand(time(NULL));
  std::mt19937_64 eng{std::random_device{}()};
  std::uniform_int_distribution<> dist{10, 100};
  const auto r = dist(eng);
  std::this_thread::sleep_for(std::chrono::milliseconds(500 + r));
  while (!stopping) {
    generate_report();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void RTCPInstance::update_stats(uint32_t rtp_packets, uint32_t sent_bytes, uint32_t rtp_ts) {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  last_rtp_timestamp = rtp_ts;
  last_ntp_timestamp = clock::ntp::now();
  our_stats.sent_pkts = rtp_packets;
  our_stats.sent_bytes += sent_bytes;
  our_stats.sent_rtp_packet = true;
}

uint32_t RTCPInstance::size_of_compound_packet(uint16_t reports,
    bool sr_packet, bool sdes_packet, bool bye_packet) const
{
    uint32_t compound_packet_size = 0;

    compound_packet_size = RTCPPackets::get_sr_packet_size(0);

    if (sdes_packet) {
      compound_packet_size += RTCPPackets::get_sdes_packet_size(ourItems_);
    }

    if (bye_packet) {
      std::vector<uint32_t> ssrcs;
      ssrcs.push_back(ssrc);
      compound_packet_size += RTCPPackets::get_bye_packet_size(ssrcs);
    }

    return compound_packet_size;
}


bool RTCPInstance::generate_report() {
    std::lock_guard<std::mutex> lock(packet_mutex_);
    if (last_ntp_timestamp == 0) {
      return false;
    }
    rtcp_pkt_sent_count_++;

    bool sr_packet = true;
    bool sdes_packet = true;
    int reports = 0;

    uint32_t compound_packet_size = size_of_compound_packet(reports, sr_packet, sdes_packet, false);
    
    uint8_t* frame = new uint8_t[compound_packet_size];
    memset(frame, 0, compound_packet_size);

    // see https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.1

    size_t write_ptr = 0;
    if (sr_packet) {
      std::lock_guard<std::mutex> lock(stats_mutex_);
        // sender reports have sender information in addition compared to receiver reports
        size_t sender_report_size = RTCPPackets::get_sr_packet_size(0);

        // This is the timestamp when the LAST rtp frame was sampled
        uint64_t sampling_ntp_ts = last_ntp_timestamp;
        uint64_t ntp_ts = clock::ntp::now();

        uint64_t diff_ms = clock::ntp::diff(sampling_ntp_ts, ntp_ts);

        uint32_t rtp_ts = last_rtp_timestamp;

        uint32_t reporting_rtp_ts = rtp_ts + (uint32_t)(diff_ms * (double(clock_rate_) / 1000));
        std::cout << "rtcp ssrc: " << ssrc << " rtp-reporting ts: " << reporting_rtp_ts << " rtp ts: " << rtp_ts << "  ntp: " << ntp_ts << std::endl;


        if (!RTCPPackets::construct_rtcp_header(frame, write_ptr, sender_report_size, reports, RTCPPackets::RTCP_FRAME_TYPE::RTCP_FT_SR) ||
            !RTCPPackets::construct_ssrc(frame, write_ptr, ssrc) ||
            !RTCPPackets::construct_sender_info(frame, write_ptr, ntp_ts, reporting_rtp_ts, our_stats.sent_pkts, our_stats.sent_bytes))
        {
            return false;
        }

        our_stats.sent_rtp_packet = false;
    }

    if (sdes_packet) {
        RTCP::rtcp_sdes_chunk chunk;
        chunk.items = ourItems_;
        chunk.ssrc = ssrc;

        // add the SDES packet after the SR/RR, mandatory, must contain CNAME
        if (!RTCPPackets::construct_rtcp_header(frame, write_ptr, RTCPPackets::get_sdes_packet_size(ourItems_), 1,
            RTCPPackets::RTCP_FRAME_TYPE::RTCP_FT_SDES) ||
            !RTCPPackets::construct_sdes_chunk(frame, write_ptr, chunk))
        {
            delete[] frame;
            return false;
        }
    }

    const auto buffer = socket->get_buffer();
    memcpy(buffer.data, frame, compound_packet_size);
    
    std::cout << "sending rtcp packet: " << compound_packet_size << std::endl;
    socket->send({ buffer.data, compound_packet_size }, 100);
    return true;
}

}  // namespace RTCP