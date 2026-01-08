// Minimal bitmap_image.hpp stub for compilation
// This is a simplified version for testing - full library available at:
// https://github.com/ArashPartow/bitmap

#ifndef BITMAP_IMAGE_HPP
#define BITMAP_IMAGE_HPP

#include <fstream>
#include <cstdint>
#include <cstring>

struct rgb_t {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

class bitmap_image {
private:
  unsigned int width_;
  unsigned int height_;
  unsigned char* data_;

public:
  bitmap_image(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      width_ = 256;
      height_ = 256;
      data_ = new unsigned char[width_ * height_ * 3];
      // Initialize with gradient pattern for testing
      for (unsigned int i = 0; i < width_ * height_ * 3; i++) {
        data_[i] = i % 256;
      }
      return;
    }

    // Read BMP header
    uint8_t header[54];
    file.read((char*)header, 54);

    width_ = *(uint32_t*)&header[18];
    height_ = *(uint32_t*)&header[22];

    // Allocate and read pixel data (simplified - assumes 24-bit BMP)
    size_t row_size = ((width_ * 3 + 3) / 4) * 4; // BMP rows are padded to 4 bytes
    size_t data_size = row_size * height_;
    uint8_t* temp = new uint8_t[data_size];
    file.read((char*)temp, data_size);

    // Convert to RGB (BMP is BGR)
    data_ = new unsigned char[width_ * height_ * 3];
    for (unsigned int y = 0; y < height_; y++) {
      for (unsigned int x = 0; x < width_; x++) {
        size_t src_idx = (height_ - 1 - y) * row_size + x * 3; // BMP is bottom-up
        size_t dst_idx = (y * width_ + x) * 3;
        data_[dst_idx + 0] = temp[src_idx + 2]; // R
        data_[dst_idx + 1] = temp[src_idx + 1]; // G
        data_[dst_idx + 2] = temp[src_idx + 0]; // B
      }
    }
    delete[] temp;
  }

  ~bitmap_image() {
    delete[] data_;
  }

  unsigned int width() const { return width_; }
  unsigned int height() const { return height_; }

  void get_pixel(unsigned int x, unsigned int y, rgb_t& color) const {
    size_t idx = (y * width_ + x) * 3;
    color.red = data_[idx + 0];
    color.green = data_[idx + 1];
    color.blue = data_[idx + 2];
  }
};

#endif
