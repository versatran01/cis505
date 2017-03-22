#ifndef CHATUTILS_H
#define CHATUTILS_H

#include <regex>
#include <string>

class AddrPort {
public:
  AddrPort() = default;
  AddrPort(const std::string &addr, int port) : addr_(addr), port_(port) {}

  const std::string &addr() const { return addr_; }
  const int port() const { return port_; }

private:
  std::string addr_;
  int port_;
};

class ServerAddrPort {
public:
  ServerAddrPort() = default;
  ServerAddrPort(const AddrPort &forward, const AddrPort &binding)
      : forward_(forward), binding_(binding) {}

private:
  AddrPort forward_;
  AddrPort binding_;
};

/**
 * @brief ParseAddrPort
 * @param addr_port String supposedly in the form addr:port
 * @return (addr, port)
 */
AddrPort ParseAddrPort(const std::string &addr_port);

std::tuple<std::string, std::string> GetForwardBinding(const std::string &line);

/**
 * @brief ParseConfig
 * @param config
 */
std::vector<ServerAddrPort> ParseConfig(const std::string &config);

#endif // CHATUTILS_H
