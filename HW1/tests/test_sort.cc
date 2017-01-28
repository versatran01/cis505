#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "mysort.h"

using VecInt = std::vector<int>;

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

TEST_CASE("Divide equal", "[divide_equal]") {}
