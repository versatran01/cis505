#include <iostream>
#include <iterator>
#include <queue>
#include <vector>

template <typename P> struct DerefGreater {
  bool operator()(const P &p1, const P &p2) {
    return *(p1.first) > *(p2.first);
  }
};

template <typename Iter> using IterPair = std::pair<Iter, Iter>;
template <typename T>
using MinHeap = std::priority_queue<T, std::vector<T>, DerefGreater<T>>;

template <typename T>
std::vector<T> k_way_merge(const std::vector<T> &data,
                           const std::vector<size_t> &split) {
  using Iter = typename std::vector<T>::const_iterator;
  const auto n = data.size();
  const auto k = split.size() - 1;
  auto beg = data.begin();

  std::vector<T> sorted;
  sorted.reserve(n);

  MinHeap<IterPair<Iter>> min_heap;
  for (size_t i = 0; i < k; ++i) {
    min_heap.push({beg + split[i], beg + split[i + 1]});
  }

  // Sorting
  for (size_t i = 0; i < n; ++i) {
    // Get the smallest item from min heap
    // Just to be safe, we acquire a copy
    IterPair<Iter> iter_pair = min_heap.top();
    // Put it into sorted
    sorted.push_back(*(iter_pair.first));
    // Pop the smallest item
    min_heap.pop();
    // Increment first iterator of iter_pair
    ++(iter_pair.first);
    // Check if it reaches last, if not, put it back in
    if (iter_pair.first != iter_pair.second) {
      min_heap.push(iter_pair);
    }
  }

  return sorted;
}

int main(int argc, char **argv) {
  std::vector<int> data = {5, 6, 7, 8, 0, 1, 9, 2, 3, 4};
  std::vector<size_t> split = {0, 4, 7, 10};
  const auto sorted = k_way_merge(data, split);
  for (const auto &s : sorted) {
    std::cout << s << " ";
  }
  std::cout << "\n";
}
