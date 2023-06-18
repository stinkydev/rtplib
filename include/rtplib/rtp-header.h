#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <WinSock2.h>

struct RtpHeader {
  uint8_t cc:4;        /* CSRC count */
  uint8_t x:1;         /* header extension flag */
  uint8_t p:1;         /* padding flag */
  uint8_t version:2;   /* protocol version */
  uint8_t pt:7;        /* payload type */
  uint8_t m:1;         /* marker bit */
  uint16_t seq:16;      /* sequence number */
  uint32_t ts;         /* timestamp */
  uint32_t ssrc;       /* synchronization source */
};

static const RtpHeader create_rtp_header(bool marker, uint8_t payload_type, uint16_t sequence_number, uint32_t timestamp, uint32_t ssrc) {
  RtpHeader header;
  header.version = 2;
  header.p = 0;
  header.x = 0;
  header.cc = 0;
  header.m = marker ? 1 : 0;
  header.pt = payload_type;
  header.seq = htons(sequence_number);
  header.ts = htonl(timestamp);
  header.ssrc = htonl(ssrc);
  return header;
}