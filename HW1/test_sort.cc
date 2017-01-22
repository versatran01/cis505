#include <algorithm>
#include <iostream>
#include <vector>

template <typename Iter, typename Comp = std::less<
                             typename std::iterator_traits<Iter>::value_type>>
void bubble_sort(Iter first, Iter last, Comp compare = Comp()) {
  // Check if Iter is random access iterator, fail if it is not
  using Iter_category = typename std::iterator_traits<Iter>::iterator_category;
  static_assert(
      std::is_same<std::random_access_iterator_tag, Iter_category>::value,
      "bubble_sort: Iter should be random access iterators or pointers to an "
      "array");

  // sort
  Iter i, j;
  for (i = first; i != last; ++i)
    for (j = first; j < i; ++j)
      if (compare(*i, *j)) std::iter_swap(i, j);
}

int main(int argc, char** argv) {
  std::vector<int> data = {5, 4, 3, 2, 1};
  bubble_sort(data.begin(), data.end());
  for (const auto& d : data) {
    std::cout << d << " ";
  }
  std::cout << "\n";
}
