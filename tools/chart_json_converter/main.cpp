#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

struct Note {
  int16_t type: 2;
  int16_t frame: 14;
  // int16_t frame;
};

std::vector<Note> left, right;

void readFile(const std::string &fileName, std::vector<Note> &vec) {
  FILE *file = fopen(fileName.c_str(), "r");
  int type, frame;
  Note note;
  while (fscanf(file, "%d %d", &type, &frame) != EOF) {
    note.type = type;
    note.frame = frame;
    if (!vec.empty() && vec.back().frame == frame) continue;
    vec.push_back(note);
  }
  fclose(file);
}

void writeBin(const std::string &name, const std::vector<Note> &vec) {
  FILE *file = fopen(name.c_str(), "w");
  fwrite(vec.data(), sizeof(Note), vec.size(), file);
  fclose(file);
}

int main(int argc, const char *argv[]) {
  if (argc < 2) return 0;
  std::string name = std::string(argv[1]);
  std::string leftFileName = name + "l.txt";
  std::string rightFileName = name + "r.txt";
  readFile(leftFileName, left);
  readFile(rightFileName, right);
  writeBin(name + "l.bin", left);
  writeBin(name + "r.bin", right);
  return 0;
}