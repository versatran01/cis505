#include <iostream>
#include <vector>

/**
 * @brief divide_equal
 * @param n size of data
 * @param k size of parts
 * @return a list of splitting indices
 */
std::vector<size_t> divide_equal(size_t n, size_t k) {
  size_t length = n / k;
  size_t remain = n % k;

  std::vector<size_t> split(k + 1);

  split[0] = 0;

  for (size_t i = 0; i < k; ++i) {
    int extra = (i < remain) ? 1 : 0;
    split[i + 1] = split[i] + length + extra;
  }

  return split;
}

int main(int agrc, char **argv) {
  std::vector<int> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  for (int i = 0; i < 5; ++i) {
    const auto split = divide_equal(data.size(), i + 1);
    for (const auto &s : split) {
      std::cout << s << " ";
    }
    std::cout << "\n";
  }
}
