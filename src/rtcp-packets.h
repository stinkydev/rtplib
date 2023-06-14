#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "rtcp-types.h"

namespace RTCP {

namespace RTCPPackets {

enum RTCP_FRAME_TYPE {
    RTCP_FT_SR   = 200, /* Sender report */
    RTCP_FT_RR   = 201, /* Receiver report */
    RTCP_FT_SDES = 202, /* Source description */
    RTCP_FT_BYE  = 203, /* Goodbye */
    RTCP_FT_APP  = 204  /* Application-specific message */
};

const uint16_t RTCP_HEADER_SIZE = 4;
const uint16_t SSRC_CSRC_SIZE = 4;
const uint16_t SENDER_INFO_SIZE = 20;
const uint16_t REPORT_BLOCK_SIZE = 24;
const uint16_t APP_NAME_SIZE = 4;

uint32_t get_sr_packet_size(uint16_t reports);
uint32_t get_rr_packet_size(uint16_t reports);
uint32_t get_bye_packet_size(const std::vector<uint32_t>& ssrcs);
uint32_t get_sdes_packet_size(const std::vector<rtcp_sdes_item>& items);

bool construct_sdes_chunk(uint8_t* frame, size_t& ptr, rtcp_sdes_chunk chunk);

// Add the RTCP header
bool construct_rtcp_header(uint8_t* frame, size_t& ptr, size_t packet_size,
    uint8_t secondField, RTCP_FRAME_TYPE frame_type);

// Add an ssrc to packet, not present in all packets
bool construct_ssrc(uint8_t* frame, size_t& ptr, uint32_t ssrc);

// Add sender info for sender report
bool construct_sender_info(uint8_t* frame, size_t& ptr,
    uint64_t ntp_ts, uint64_t rtp_ts, uint32_t sent_packets, uint32_t sent_bytes);

// Add one report block for sender or receiver report
bool construct_report_block(uint8_t* frame, size_t& ptr, uint32_t ssrc, uint8_t fraction,
    uint32_t dropped_packets, uint16_t seq_cycles, uint16_t max_seq, uint32_t jitter,
    uint32_t lsr, uint32_t dlsr);

// Add BYE ssrcs, should probably be removed
bool construct_bye_packet(uint8_t* frame, size_t& ptr, const std::vector<uint32_t>& ssrcs);
}
}
