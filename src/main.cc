#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

#include "rtp-header.h"
#include "rtp-session.h"
#include "utils.h"

#include "testing/ffmpeg-reader.h"
#include "testing/ffmpeg-encoder.h"

int main() {
  uint32_t ts = 0;
  const auto frame_dur = 90000 / 25;

  const RtpSession rtp_session;
  const std::shared_ptr<RtpStream> rtp_stream = std::make_shared<RtpStream>("127.0.0.1", 20000, 20002, 1111, 101);

  FFmpegEncoder encoder;
  encoder.on_packet_encoded = [&](AVPacket* pkt, uint32_t pts) {
    std::cout << "packet: " << pkt->size << " pts: " << pts << std::endl;
    rtp_stream->send(pkt->data, pkt->size, ts);
    ts += frame_dur;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  };

  FFmpegReader reader("c:\\output.mp4");
  reader.on_video_frame = [&](AVFrame* frame, uint8_t* data, size_t size) {
    encoder.encode(frame);
  };

  reader.decode(); 

/*
  for (size_t i = 0; i < 1; i++) {
    char filename[128];
    std::sprintf(filename, "../../test-data/h264-1280x720-%06zd.bin", i);
    std::cout << filename << std::endl;
    if (file_exists(filename)) {
      const auto data = load_test_file(filename);

      rtp_stream->send(data.data(), data.size(), ts);
      ts += frame_dur;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }
  */

  return 0;
}