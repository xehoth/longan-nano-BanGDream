#include <cstdio>
#include <cstdint>
#include <iostream>

struct Note {
  int16_t type: 2;
  int16_t frame: 14;
} buf[1000];

int main() {
  FILE *file = fopen("128.normall.bin", "r");
  fread(buf, 1, 278, file);
  std::cerr << buf[0].type;
  fclose(file);
}