#pragma once

#include <stdint.h>
#include <vector>

class RtpHeader {
 private:
  uint8_t version_;
  uint8_t padding_;
  uint8_t extension_;
  uint8_t csrc_count_;
  uint8_t marker_;
  uint8_t payload_type_;
  uint16_t sequence_number_;
  uint32_t timestamp_;
  uint32_t ssrc_; 
 public:
  RtpHeader();
  RtpHeader(const std::vector<uint8_t>& bytes);
  ~RtpHeader();


  void set_version(uint8_t version);
  void set_padding(uint8_t padding);
  void set_extension(uint8_t extension);
  void set_csrc_count(uint8_t csrc_count);
  void set_marker(uint8_t marker);
  void set_payload_type(uint8_t payload_type);
  void set_sequence_number(uint16_t sequence_number);
  void set_timestamp(uint32_t timestamp);
  void set_ssrc(uint32_t ssrc);

  uint8_t get_version() const;
  uint8_t get_padding() const;
  uint8_t get_extension() const;
  uint8_t get_csrc_count() const;
  uint8_t get_marker() const;
  uint8_t get_payload_type() const;
  uint16_t get_sequence_number() const;
  uint32_t get_timestamp() const;
  uint32_t get_ssrc() const; 

  std::vector<uint8_t> get_bytes() const;
};