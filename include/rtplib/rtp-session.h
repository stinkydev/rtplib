#pragma once

#include <vector>
#include <memory>
#include "rtp-stream.h"

class RtpSession {
 private:
  std::vector<std::shared_ptr<RtpStream>> streams;
 public:
  void add_stream(std::shared_ptr<RtpStream> stream) {
    streams.push_back(stream);
  }
};
