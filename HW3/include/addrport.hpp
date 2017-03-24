#ifndef ADDRPORT_HPP
#define ADDRPORT_HPP

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

  const std::string full() const { return addr_ + ":" + std::to_string(port_); }
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

  const AddrPort &forward() const { return forward_; }
  const AddrPort &binding() const { return binding_; }

private:
  AddrPort forward_;
  AddrPort binding_;
};

#endif // ADDRPORT_HPP
