#include "chatutils.hpp"

std::tuple<std::string, int> ParseAddrAndPort(const std::string &addr_port) {
  static const char *pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_port_regex(pattern);

  std::smatch results;
  if (!std::regex_search(addr_port, results, addr_port_regex)) {
    throw std::invalid_argument("Ivalid address and port");
  }

  // Port must be a number because of the regex
  return std::make_tuple(results.str(1), std::atoi(results.str(2).c_str()));
}
