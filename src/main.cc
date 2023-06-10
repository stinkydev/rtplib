#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "rtp-header.h"
#include "rtp-session.h"
#include "utils.h"

int main() {
  const RtpSession rtp_session();
  const std::shared_ptr<RtpStream> rtp_stream = std::make_shared<RtpStream>("127.0.0.1", 20000, 20002, 1111, 101);

  const std::string filename = "../test-data/h264-1280x720-000000.bin";
  std::cout << "Loading test file: " << filename << std::endl;
  if (file_exists(filename)) {
    const auto data = load_test_file(filename);
    std::cout << "Loaded " << data.size() << " bytes" << std::endl;
    rtp_stream->send(data.data(), data.size());
  }

  return 0;
}