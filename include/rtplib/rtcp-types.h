#pragma once

#include <stdint.h>
#include <vector>

namespace RTCP {
  struct rtcp_header {
    /** \brief  This field identifies the version of RTP. The version defined by
     * RFC 3550 is two (2).  */
    uint8_t version = 0;
    /** \brief Does this packet contain padding at the end */
    uint8_t padding = 0;
    union {
        /** \brief Source count or report count. Alternative to pkt_subtype. */
        uint8_t count = 0;   
        /** \brief Subtype in APP packets. Alternative to count */
        uint8_t pkt_subtype; 
    };
    /** \brief Identifies the RTCP packet type */
    uint8_t pkt_type = 0;
    /** \brief Length of the whole message measured in 32-bit words */
    uint16_t length = 0;
};

/** \brief See <a href="https://www.rfc-editor.org/rfc/rfc3550#section-6.4.1" target="_blank">RFC 3550 section 6.4.1</a> */
struct rtcp_sender_info {
    /** \brief NTP timestamp, most significant word */
    uint32_t ntp_msw = 0;
    /** \brief NTP timestamp, least significant word */
    uint32_t ntp_lsw = 0;
    /** \brief RTP timestamp corresponding to this NTP timestamp */
    uint32_t rtp_ts = 0;
    uint32_t pkt_cnt = 0;
    /** \brief Also known as octet count*/
    uint32_t byte_cnt = 0;
};

struct rtcp_report_block {
    uint32_t ssrc = 0;
    uint8_t  fraction = 0;
    int32_t  lost = 0;
    uint32_t last_seq = 0;
    uint32_t jitter = 0;
    uint32_t lsr = 0;  /* last Sender Report */
    uint32_t dlsr = 0; /* delay since last Sender Report */
};

struct rtcp_sender_report {
    struct rtcp_header header;
    uint32_t ssrc = 0;
    struct rtcp_sender_info sender_info;
    std::vector<rtcp_report_block> report_blocks;
};

struct rtcp_sdes_item {
    uint8_t type = 0;
    uint8_t length = 0;
    uint8_t *data = nullptr;
};

/** \brief See <a href="https://www.rfc-editor.org/rfc/rfc3550#section-6.5" target="_blank">RFC 3550 section 6.5</a> */
struct rtcp_sdes_chunk {
    uint32_t ssrc = 0;
    std::vector<rtcp_sdes_item> items;
};

/** \brief See <a href="https://www.rfc-editor.org/rfc/rfc3550#section-6.5" target="_blank">RFC 3550 section 6.5</a> */
struct rtcp_sdes_packet {
    struct rtcp_header header;
    std::vector<rtcp_sdes_chunk> chunks;
};
 
}