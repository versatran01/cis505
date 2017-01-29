#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "mysort.h"

using VecInt = std::vector<int>;
using VecStr = std::vector<std::string>;
using VecSizeT = std::vector<size_t>;

TEST_CASE("Bubble sort", "[BubbleSort]") {
  SECTION("Sort a sorted array") {
    VecInt d = {1, 2, 3};
    BubbleSort(d.begin(), d.end());
    VecInt s = {1, 2, 3};
    REQUIRE(s == d);
  }

  SECTION("Sort an unsorted array") {
    VecInt d = {3, 2, 1};
    BubbleSort(d.begin(), d.end());
    VecInt s = {1, 2, 3};
    REQUIRE(s == d);
  }

  SECTION("Sort with dffferent iterators") {
    VecInt d = {3, 2, 1};
    BubbleSort(d.begin(), d.begin() + 2);
    VecInt s = {2, 3, 1};
    REQUIRE(s == d);
  }

  SECTION("Sort with same iterators") {
    VecInt d = {3, 2, 1};
    BubbleSort(d.begin(), d.begin());
    VecInt s = {3, 2, 1};
    REQUIRE(s == d);
  }

  SECTION("Sort with raw pointers") {
    VecInt d = {3, 2, 1};
    BubbleSort(&d[0], &d[d.size()]);
    VecInt s = {1, 2, 3};
    REQUIRE(s == d);
  }

  SECTION("Sort with strings") {
    VecStr d = {"1", "10", "2", "11"};
    BubbleSort(d.begin(), d.end());
    VecStr s = {"1", "10", "11", "2"};
    REQUIRE(s == d);
  }
}

TEST_CASE("Divide equal", "[DivideEqual]") {
  SECTION("Divide evenly") {
    const auto d = DivideEqual(4, 2);
    VecSizeT s = {0, 2, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide unevenly") {
    const auto d = DivideEqual(4, 3);
    VecSizeT s = {0, 2, 3, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide by 1") {
    const auto d = DivideEqual(4, 1);
    VecSizeT s = {0, 4};
    REQUIRE(s == d);
  }

  SECTION("Divide by n") {
    const auto d = DivideEqual(4, 4);
    VecSizeT s = {0, 1, 2, 3, 4};
    REQUIRE(s == d);
  }
}

TEST_CASE("Merge sort", "[MergeSort]") {
  SECTION("Two way merge sort") {
    VecInt d = {3, 4, 1, 2};
    const auto s = DivideEqual(d.size(), 2);
    const auto m = MergeSort(d, s);
    VecInt r = {1, 2, 3, 4};
    REQUIRE(m == r);
  }

  SECTION("K way merge sort") {
    VecInt d = {3, 4, 5, 0, 1, 2, 6, 7, 8};
    const auto s = DivideEqual(d.size(), 3);
    const auto m = MergeSort(d, s);
    VecInt r = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    REQUIRE(m == r);
  }
}
