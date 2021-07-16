#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>

union Color {
  struct {
    uint16_t b : 5;
    uint16_t a : 1;
    uint16_t g : 5;
    uint16_t r : 5;
  };
  uint16_t u16;
  uint8_t u8[2];

  Color() : u16() {}
  Color(uint16_t u16) : u16(u16) {}
  Color(int r, int g, int b, int a = 1) : b(b), a(a), g(g), r(r) {}
};

int main() {
  std::string file = "2menu";
  int w, h, chs;
  stbi_uc *data = stbi_load((file + ".png").c_str(), &w, &h, &chs, 0);
  std::vector<Color> buf(w * h);
  // std::cerr << w << " " << h << " " << chs << std::endl;
  std::vector<Color> outBuf(w * h);
  if (chs == 4) {
    for (int i = 0; i < w * h; ++i) {
      double r = data[i * 4 + 0] / 255.0;
      double g = data[i * 4 + 1] / 255.0;
      double b = data[i * 4 + 2] / 255.0;
      double a = data[i * 4 + 3] / 255.0;
      buf[i].r = r * a * 31;
      buf[i].g = g * a * 31;
      buf[i].b = b * a * 31;
      buf[i].a = a > 1e-3;
    }
  } else if (chs == 3) {
    for (int i = 0; i < w * h; ++i) {
      double r = data[i * 3 + 0] / 255.0;
      double g = data[i * 3 + 1] / 255.0;
      double b = data[i * 3 + 2] / 255.0;
      buf[i].r = r * 31;
      buf[i].g = g * 31;
      buf[i].b = b * 31;
      buf[i].a = 1;
    }
  }
  stbi_image_free(data);
  FILE *output = fopen((file + ".bin").c_str(), "w");
  for (int x = 0; x < 80; ++x) {
    for (int y = 0; y < 160; ++y) {
      outBuf[(80 - x - 1) * 160 + (160 - 1 - y)] = buf[(160 - y - 1) * 80 +
      x];
    }
  }
  fwrite(outBuf.data(), sizeof(Color), w * h, output);
  fclose(output);
}