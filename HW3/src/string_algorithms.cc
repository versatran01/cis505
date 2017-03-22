#include "string_algorithms.hpp"

#include <algorithm>
#include <locale>

bool begins_with(const std::string &s, const std::string &begin) {
  if (begin.size() > s.size())
    return false;
  return std::equal(begin.begin(), begin.end(), s.begin());
}

bool ends_with(const std::string &s, const std::string &end) {
  if (end.size() > s.size())
    return false;
  return std::equal(end.rbegin(), end.rend(), s.rbegin());
}

void to_upper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

void to_lower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

std::string to_upper_copy(std::string s) {
  to_upper(s);
  return s;
}

std::string to_lower_copy(std::string s) {
  to_lower(s);
  return s;
}

void trim_front(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
}

void trim_back(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
}

void trim(std::string &s) {
  trim_front(s);
  trim_back(s);
}

std::string trim_front_copy(std::string s) {
  trim_front(s);
  return s;
}

std::string trim_back_copy(std::string s) {
  trim_back(s);
  return s;
}

std::string trim_copy(std::string s) {
  trim(s);
  return s;
}
