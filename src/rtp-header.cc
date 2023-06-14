#include <rtplib/rtp-header.h>

RtpHeader::RtpHeader() {
  bytes_buf = std::make_unique<std::vector<uint8_t>>();

  version_ = 2;
  padding_ = 0;
  extension_ = 0;
  csrc_count_ = 0;
  marker_ = 0;
  payload_type_ = 0;
  sequence_number_ = 0;
  timestamp_ = 0;
  ssrc_ = 0;
}

RtpHeader::RtpHeader(const std::vector<uint8_t>& bytes) {
  bytes_buf = std::make_unique<std::vector<uint8_t>>();
  // Extract the RTP version from the first byte.
  version_ = bytes[0] >> 6;

  // Extract the padding flag from the first byte.
  padding_ = bytes[0] >> 5 & 1;

  // Extract the extension flag from the first byte.
  extension_ = bytes[0] >> 4 & 1;

  // Extract the CSRC count from the first byte.
  csrc_count_ = bytes[0] & 0xF;

  // Extract the marker bit from the second byte.
  marker_ = bytes[1] >> 7;

  // Extract the payload type from the second byte.
  payload_type_ = bytes[1] & 0x7F;

  // Extract the sequence number from the third and fourth bytes.
  sequence_number_ = (bytes[2] << 8) | bytes[3];

  // Extract the timestamp from the fifth through eighth bytes.
  timestamp_ = (bytes[4] << 24) | (bytes[5] << 16) | (bytes[6] << 8) | bytes[7];

  // Extract the SSRC from the ninth through twelfth bytes.
  ssrc_ = (bytes[8] << 24) | (bytes[9] << 16) | (bytes[10] << 8) | bytes[11];
}

RtpHeader::~RtpHeader() {
}

void RtpHeader::set_version(uint8_t version) {
  version_ = version;
}

void RtpHeader::set_padding(uint8_t padding) {
  padding_ = padding;
}

void RtpHeader::set_extension(uint8_t extension) {
  extension_ = extension;
}

void RtpHeader::set_csrc_count(uint8_t csrc_count) {
  csrc_count_ = csrc_count;
}

void RtpHeader::set_marker(uint8_t marker) {
  marker_ = marker;
}

void RtpHeader::set_payload_type(uint8_t payload_type) {
  payload_type_ = payload_type;
}

void RtpHeader::set_sequence_number(uint16_t sequence_number) {
  sequence_number_ = sequence_number;
}

void RtpHeader::set_timestamp(uint32_t timestamp) {
  timestamp_ = timestamp;
}

void RtpHeader::set_ssrc(uint32_t ssrc) {
  ssrc_ = ssrc;
}

uint8_t RtpHeader::get_version() const {
  return version_;
}

uint8_t RtpHeader::get_padding() const {
  return padding_;
}

uint8_t RtpHeader::get_extension() const {
  return extension_;
}

uint8_t RtpHeader::get_csrc_count() const {
  return csrc_count_;
}

uint8_t RtpHeader::get_marker() const {
  return marker_;
}

uint8_t RtpHeader::get_payload_type() const {
  return payload_type_;
}

uint16_t RtpHeader::get_sequence_number() const {
  return sequence_number_;
}

uint32_t RtpHeader::get_timestamp() const {
  return timestamp_;
}

uint32_t RtpHeader::get_ssrc() const {
  return ssrc_;
}

std::vector<uint8_t> RtpHeader::get_bytes() const {
  const auto bytes = bytes_buf.get();
  bytes->resize(12);
  bytes->at(0) = version_ << 6 | padding_ << 5 | extension_ << 4 | csrc_count_;
  bytes->at(1) = marker_ << 7 | payload_type_;
  bytes->at(2) = sequence_number_ >> 8;
  bytes->at(3) = sequence_number_ & 0xFF;
  bytes->at(4) = timestamp_ >> 24;
  bytes->at(5) = timestamp_ >> 16 & 0xFF;
  bytes->at(6) = timestamp_ >> 8 & 0xFF;
  bytes->at(7) = timestamp_ & 0xFF;
  bytes->at(8) = ssrc_ >> 24;
  bytes->at(9) = ssrc_ >> 16 & 0xFF;
  bytes->at(10) = ssrc_ >> 8 & 0xFF;
  bytes->at(11) = ssrc_ & 0xFF;
  return *bytes;
}