#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "mysort.h"

using VecInt = std::vector<int>;
using VecSizeT = std::vector<size_t>;

TEST_CASE("Bubble sort", "[bubble_sort]") {
  SECTION("Sort a sorted array") {
    VecInt d = {1, 2, 3};
    bubble_sort(d.begin(), d.end());
    VecInt s = {1, 2, 3};
    REQUIRE(s == d);
  }

  SECTION("Sort an unsorted array") {
    VecInt d = {3, 2, 1};
    bubble_sort(d.begin(), d.end());
    VecInt s = {1, 2, 3};
    REQUIRE(s == d);
  }

  SECTION("Sort with dffferent iterators") {
    VecInt d = {3, 2, 1};
    bubble_sort(d.begin(), d.begin() + 2);
    VecInt s = {2, 3, 1};
    REQUIRE(s == d);
  }

  SECTION("Sort with same iterators") {
    VecInt d = {3, 2, 1};
    bubble_sort(d.begin(), d.begin());
    VecInt s = {3, 2, 1};
    REQUIRE(s == d);
  }
}

TEST_CASE("Divide equal", "[divide_equal]") {
  SECTION("Divide evenly") {
    const auto d = divide_equal(4, 2);
    VecSizeT s = {0, 2, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide unevenly") {
    const auto d = divide_equal(4, 3);
    VecSizeT s = {0, 2, 3, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide by 1") {
    const auto d = divide_equal(4, 1);
    VecSizeT s = {0, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide by n") {
    const auto d = divide_equal(4, 4);
    VecSizeT s = {0, 1, 2, 3, 4};
    REQUIRE(s == d);
  }
}

TEST_CASE("Merge sort", "[merge_sort]") {
  SECTION("Two way merge sort") {
    VecInt d = {3, 4, 1, 2};
    const auto s = divide_equal(d.size(), 2);
    const auto m = k_way_merge(d, s);
    VecInt r = {1, 2, 3, 4};
    REQUIRE(m == r);
  }

  SECTION("k way merge sort") {
    VecInt d = {3, 4, 5, 0, 1, 2, 6, 7, 8};
    const auto s = divide_equal(d.size(), 3);
    const auto m = k_way_merge(d, s);
    VecInt r = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    REQUIRE(m == r);
  }
}
