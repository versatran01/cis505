#include "chatutils.hpp"

#include "loguru.hpp"
#include "string_algorithms.hpp"

AddrPort ParseAddrPort(const std::string &addr_port) {
  static const char *pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_port_regex(pattern);

  std::smatch results;
  if (!std::regex_search(addr_port, results, addr_port_regex)) {
    throw std::invalid_argument("Ivalid address and port");
  }

  // Port must be a number because of the regex
  return {results.str(1), std::atoi(results.str(2).c_str())};
}

std::tuple<std::string, std::string>
GetForwardBinding(const std::string &line) {
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

sockaddr_in MakeSockAddrInet(const AddrPort &addr_port) {
  return MakeSockAddrInet(addr_port.addr(), addr_port.port());
}

sockaddr_in MakeSockAddrInet(const std::string &addr, int port) {
  struct sockaddr_in addr_inet;
  bzero(&addr_inet, sizeof(addr_inet));
  addr_inet.sin_family = AF_INET;
  addr_inet.sin_addr.s_addr = inet_addr(addr.c_str());
  addr_inet.sin_port = htons(port);
  return addr_inet;
}

AddrPort GetAddrPort(const sockaddr_in &sock_addr) {
  std::string addr(inet_ntoa(sock_addr.sin_addr));
  int port = ntohs(sock_addr.sin_port);
  return {addr, port};
}
