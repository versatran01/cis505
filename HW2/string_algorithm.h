#ifndef STRING_ALGORITHM_H
#define STRING_ALGORITHM_H

#include <algorithm>
#include <string>

static void to_upper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

static void to_lower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

static std::string to_upper_copy(std::string s) {
  to_upper(s);
  return s;
}

static std::string to_lower_copy(std::string s) {
  to_lower(s);
  return s;
}

#endif // STRING_ALGORITHM_H
