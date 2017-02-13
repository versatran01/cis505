#include <experimental/filesystem>
#include <iostream>
#include <iostream>

namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {
  fs::path cwd(fs::current_path());
  std::cout << cwd << std::endl;
}
