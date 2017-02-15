#ifndef STRING_ALGORITHM_H
#define STRING_ALGORITHM_H

#include <string>

/**
 * @brief to_upper
 * @param s
 */
void to_upper(std::string &s);

/**
 * @brief to_lower
 * @param s
 */
void to_lower(std::string &s);

/**
 * @brief to_upper_copy
 * @param s
 * @return
 */
std::string to_upper_copy(std::string s);

/**
 * @brief to_lower_copy
 * @param s
 * @return
 */
std::string to_lower_copy(std::string s);

/**
 * @brief trim_front
 * @param s
 */
void trim_front(std::string &s);

/**
 * @brief trim_back
 * @param s
 */
void trim_back(std::string &s);

/**
 * @brief trim
 * @param s
 */
void trim(std::string &s);

/**
 * @brief trim_front_copy
 * @param s
 * @return
 */
std::string trim_front_copy(std::string s);

/**
 * @brief trim_back_copy
 * @param s
 * @return
 */
std::string trim_back_copy(std::string s);

/**
 * @brief trim_copy
 * @param s
 * @return
 */
std::string trim_copy(std::string s);

#endif // STRING_ALGORITHM_H
