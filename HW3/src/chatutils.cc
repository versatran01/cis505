#include "chatutils.hpp"

#include "loguru.hpp"
#include "string_algorithms.hpp"

Address ParseAddress(const std::string &addr_str) {
  static const char *pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_regex(pattern);

  std::smatch results;
  if (!std::regex_search(addr_str, results, addr_regex)) {
    throw std::invalid_argument("Ivalid address and port");
  }

  // Port must be a number because of the regex
  return {results.str(1), std::atoi(results.str(2).c_str())};
}

std::tuple<std::string, std::string> GetServerAddress(const std::string &line) {
  const auto delim_index = line.find(',');
  if (delim_index == std::string::npos) {
    // Same address
    return std::make_tuple(line, line);
  } else {
    // Different addresses
    return std::make_tuple(trim_copy(line.substr(0, delim_index)),
                           trim_copy(line.substr(delim_index + 1)));
  }
}

sockaddr_in MakeSockAddrInet(const Address &addr) {
  return MakeSockAddrInet(addr.addr(), addr.port());
}

sockaddr_in MakeSockAddrInet(const std::string &addr, int port) {
  struct sockaddr_in addr_inet;
  bzero(&addr_inet, sizeof(addr_inet));
  addr_inet.sin_family = AF_INET;
  addr_inet.sin_addr.s_addr = inet_addr(addr.c_str());
  addr_inet.sin_port = htons(port);
  return addr_inet;
}

Address MakeAddress(const sockaddr_in &sock_addr) {
  std::string addr(inet_ntoa(sock_addr.sin_addr));
  int port = ntohs(sock_addr.sin_port);
  return {addr, port};
}
