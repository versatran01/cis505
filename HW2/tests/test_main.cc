#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "string_algorithm.h"

TEST_CASE("To upper copy", "[to_upper_copy]") {
  SECTION("To upper same") {
    const auto result = to_upper_copy("ABC");
    REQUIRE(result == "ABC");
  }

  SECTION("To upper all lower") {
    const auto result = to_upper_copy("abc");
    REQUIRE(result == "ABC");
  }

  SECTION("To upper mixed") {
    const auto result = to_upper_copy("Abc");
    REQUIRE(result == "ABC");
  }

  SECTION("To upper special") {
    const auto result = to_upper_copy("Abc +-*/.");
    REQUIRE(result == "ABC +-*/.");
  }
}

TEST_CASE("To lower copy", "[to_lower_copy]") {
  SECTION("To lower same") {
    const auto result = to_lower_copy("abc");
    REQUIRE(result == "abc");
  }

  SECTION("To lower all upper") {
    const auto result = to_lower_copy("ABC");
    REQUIRE(result == "abc");
  }

  SECTION("To lower mixed") {
    const auto result = to_lower_copy("Abc");
    REQUIRE(result == "abc");
  }

  SECTION("To lower special") {
    const auto result = to_lower_copy("Abc +-*/.");
    REQUIRE(result == "abc +-*/.");
  }
}

TEST_CASE("Trim", "[trim]") {
  SECTION("Trim front with no space") {
    const auto result = trim_front_copy("abc");
    REQUIRE(result == "abc");
  }

  SECTION("Trim front with space") {
    const auto result = trim_front_copy("   abc");
    REQUIRE(result == "abc");
  }

  SECTION("Trim back with space") {
    const auto result = trim_back_copy("abc   ");
    REQUIRE(result == "abc");
  }

  SECTION("Trim back with no space") {
    const auto result = trim_back_copy("   abc");
    REQUIRE(result == "   abc");
  }

  SECTION("Trim both") {
    const auto result = trim_copy("   abc   ");
    REQUIRE(result == "abc");
  }
}
