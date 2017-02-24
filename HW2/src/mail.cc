#include "mail.h"

#include <algorithm>
#include <iomanip>

Mail::Mail(const std::string &sender) : sender_(sender) {}

void Mail::Clear() {
  sender_.clear();
  recipients_.clear();
  data_.clear();
}

void Mail::SetTimeFromString(const std::string &time_str) {
  std::tm tm = {};
  std::stringstream ss(time_str);
  ss >> std::get_time(&tm, "%a %b %d %H:%M:%S %Y");
  time_ = std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

bool Mail::RecipientExists(const std::string &mailaddr) const {
  return std::find(recipients_.begin(), recipients_.end(), mailaddr) !=
         recipients_.end();
}

bool Mail::Empty() const {
  return sender_.empty() && recipients_.empty() && data_.empty();
}

std::string Mail::TimeStr() const {
  const auto time = std::chrono::system_clock::to_time_t(time_);
  return std::string(std::ctime(&time));
}

size_t Mail::Octets() const {
  size_t n = 0;
  for (const auto &line : data_) {
    n += (line.size() + 2);
  }
  return n;
}
