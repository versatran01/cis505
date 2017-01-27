#ifndef MYSORT_H
#define MYSORT_H

#include <algorithm>
#include <iterator>
#include <vector>

/**
 * @brief bubble_sort
 * @param first Iterator to the first element in range
 * @param last Iterator to the last element in range
 * @param compare Functor that follows strict weak ordering
 */
template <typename Iter, typename Comp = std::less<
                             typename std::iterator_traits<Iter>::value_type>>
void bubble_sort(Iter first, Iter last, Comp compare = Comp()) {
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
      if (compare(*i, *j))
        std::iter_swap(i, j);
}

/**
 * @brief divide_equal
 * @param n size of data
 * @param k size of parts
 * @return a list of splitting indices
 */
static std::vector<size_t> divide_equal(size_t n, size_t k) {
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

#endif // MYSORT_H
