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
#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>

// trim from start (in place)
static void trim_front(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
}

// trim from end (in place)
static void trim_back(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
}

static void trim(std::string &s) {
  trim_front(s);
  trim_back(s);
}

static std::string trim_front_copy(std::string s) {
  trim_front(s);
  return s;
}

static std::string trim_back_copy(std::string s) {
  trim_back(s);
  return s;
}

static std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

#endif // STRING_ALGORITHM_H
