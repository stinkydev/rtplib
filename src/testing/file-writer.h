#pragma once

class FileWriter {
 private:
   std::ofstream file_stream;
   std::mutex mtx;
 public:
  void write(const char* data, size_t size) {
    std::lock_guard<std::mutex> m(mtx);
    file_stream.write(data, size);
    file_stream.flush();
  }

  FileWriter(const std::string filename) {
    std::lock_guard<std::mutex> m(mtx);
    file_stream.open(filename, std::ios::out | std::ios::binary);
  }

  ~FileWriter() {
    std::lock_guard<std::mutex> m(mtx);
    file_stream.close();
  }
};
