#include <string>
#include <fstream>
#include <streambuf>

std::string textFileRead(const std::string &filename) {
  auto in = std::ifstream(filename);
  std::string str((std::istreambuf_iterator<char>(in)),
                   std::istreambuf_iterator<char>());
  return str;
}
