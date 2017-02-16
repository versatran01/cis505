#ifndef STRING_ALGORITHM_H
#define STRING_ALGORITHM_H

#include <string>

// Convert string to upper or lower
void to_upper(std::string &s);
void to_lower(std::string &s);
std::string to_upper_copy(std::string s);
std::string to_lower_copy(std::string s);

// Trim whitespaces from string
void trim_front(std::string &s);
void trim_back(std::string &s);
void trim(std::string &s);
std::string trim_front_copy(std::string s);
std::string trim_back_copy(std::string s);
std::string trim_copy(std::string s);

#endif // STRING_ALGORITHM_H
