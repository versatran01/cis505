#ifndef MYSORT_H
#define MYSORT_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <queue>
#include <string>
#include <vector>

/**
 * @brief Bubble sort with range
 * @param first Iterator to the first element in range
 * @param last Iterator to the last element in range
 * @param compare Functor that follows strict weak ordering
 */
template <
    typename Iter,
    typename Comp = std::less<typename std::iterator_traits<Iter>::value_type>>
void BubbleSort(Iter first, Iter last, Comp compare = Comp()) {
  // Check if Iter is random access iterator, fail at compile time if it is not
  // This part is totally useless, just for fun
  using Iter_category = typename std::iterator_traits<Iter>::iterator_category;
  static_assert(
      std::is_same<std::random_access_iterator_tag, Iter_category>::value,
      "bubble_sort: Iter should be random access iterators or pointers to an "
      "array");

  // sort routine
  Iter i, j;
  for (i = first; i != last; ++i)
    for (j = first; j < i; ++j)
      if (compare(*i, *j)) std::iter_swap(i, j);
}

/**
 * @brief Divide a size n into k almost equal parts
 * @param n size of data
 * @param k size of parts
 * @return a list of splitting indices, size k+1, starts from 0, ends in n
 */
static std::vector<size_t> DivideEqual(size_t n, size_t k) {
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

/// Custom comparator for min heap
template <typename P>
struct DerefFirstGreater {
  bool operator()(const P &p1, const P &p2) {
    return *(p1.first) > *(p2.first);
  }
};

template <typename Iter>
using IterPair = std::pair<Iter, Iter>;
template <typename T>
using MinHeap = std::priority_queue<T, std::vector<T>, DerefFirstGreater<T>>;

/**
 * @brief A k way merge sort using min heap
 * @param data Input data
 * @param split Split from DivideEqual
 * @return vector with merged data, sorted
 */
template <typename T>
std::vector<T> MergeSort(const std::vector<T> &data,
                         const std::vector<size_t> &split) {
  // Since input is a vector, Iter is guaranteed to be a random access iterator
  using Iter = typename std::vector<T>::const_iterator;

  const auto n = data.size();
  const auto k = split.size() - 1;
  assert(n >= k && "n must be greater than k");
  auto beg = data.begin();

  // Prepare output
  std::vector<T> merged;
  merged.reserve(n);

  // Prepare min heap
  MinHeap<IterPair<Iter>> min_heap;
  for (size_t i = 0; i < k; ++i) {
    min_heap.push({beg + split[i], beg + split[i + 1]});
  }

  // Sorting
  for (size_t i = 0; i < n; ++i) {
    // Get the smallest item from min heap
    // Just to be safe, we acquire a copy, could possible use auto?
    IterPair<Iter> iter_pair = min_heap.top();
    // Put it into merged
    merged.push_back(*(iter_pair.first));
    // Pop the smallest item
    min_heap.pop();
    // Increment first iterator of iter_pair
    ++(iter_pair.first);
    // Check if it reaches last, if not, put it back in the heap
    if (iter_pair.first != iter_pair.second) {
      min_heap.push(iter_pair);
    }
  }

  assert(merged.size() == data.size() && "merged size doesn't match data size");
  return merged;
}

/**
 * @brief Print a range to stdout
 */
template <typename Iter>
void PrintRangeToStdout(Iter first, Iter last, const char *delim = "\n") {
  using T = typename std::iterator_traits<Iter>::value_type;
  std::copy(first, last, std::ostream_iterator<T>(std::cout, delim));
}

/**
 * @brief Read data from files and put into a single vector
 * @param files
 * @return
 */
template <typename T>
std::vector<T> ReadDataFromFiles(const std::vector<std::string> &files) {
  // Read data from files
  std::vector<T> data;
  for (const auto &f : files) {
    std::ifstream infile(f);
    std::istream_iterator<T> input(infile);
    std::copy(input, std::istream_iterator<T>(), std::back_inserter(data));
  }

  return data;
}

#endif  // MYSORT_H
