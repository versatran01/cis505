#ifndef STRING_ALGORITHM_H
#define STRING_ALGORITHM_H

#include <algorithm>
#include <locale>
#include <string>

/**
 * @brief to_upper
 * @param s
 */
static void to_upper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

/**
 * @brief to_lower
 * @param s
 */
static void to_lower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

/**
 * @brief to_upper_copy
 * @param s
 * @return
 */
static std::string to_upper_copy(std::string s) {
  to_upper(s);
  return s;
}

/**
 * @brief to_lower_copy
 * @param s
 * @return
 */
static std::string to_lower_copy(std::string s) {
  to_lower(s);
  return s;
}

/**
 * @brief trim_front
 * @param s
 */
static void trim_front(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), is_not_space));
}

/**
 * @brief trim_back
 * @param s
 */
static void trim_back(std::string &s) {
  auto is_not_space = [](const unsigned char &s) { return !std::isspace(s); };
  s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
}

/**
 * @brief trim
 * @param s
 */
static void trim(std::string &s) {
  trim_front(s);
  trim_back(s);
}

/**
 * @brief trim_front_copy
 * @param s
 * @return
 */
static std::string trim_front_copy(std::string s) {
  trim_front(s);
  return s;
}

/**
 * @brief trim_back_copy
 * @param s
 * @return
 */
static std::string trim_back_copy(std::string s) {
  trim_back(s);
  return s;
}

/**
 * @brief trim_copy
 * @param s
 * @return
 */
static std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

#endif // STRING_ALGORITHM_H
