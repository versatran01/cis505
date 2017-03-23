#ifndef CHATUTILS_H
#define CHATUTILS_H

#include <arpa/inet.h>
#include <regex>
#include <string>

class AddrPort {
public:
  AddrPort() = default;
  AddrPort(const std::string &addr, int port) : addr_(addr), port_(port) {}

  inline bool operator==(const AddrPort &other) const {
    return addr_ == other.addr() && port_ == other.port();
  }
  inline bool operator!=(const AddrPort &other) const {
    return !(*this == other);
  }

  const std::string &addr() const { return addr_; }
  const int port() const { return port_; }
  const std::string addr_port() const {
    return addr_ + ":" + std::to_string(port_);
  }

private:
  std::string addr_;
  int port_;
};

class ServerAddrPort {
public:
  ServerAddrPort() = default;
  ServerAddrPort(const AddrPort &forward, const AddrPort &binding)
      : forward_(forward), binding_(binding) {}

  const AddrPort &forward() const { return forward_; }
  const AddrPort &binding() const { return binding_; }

private:
  AddrPort forward_;
  AddrPort binding_;
};

AddrPort ParseAddrPort(const std::string &addr_port);
std::tuple<std::string, std::string> GetForwardBinding(const std::string &line);

sockaddr_in MakeSockAddrInet(const AddrPort &addr_port);
sockaddr_in MakeSockAddrInet(const std::string &addr, int port);
AddrPort GetAddrPort(const sockaddr_in &sock_addr);

#endif // CHATUTILS_H
