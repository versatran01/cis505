#ifndef ADDRPORT_HPP
#define ADDRPORT_HPP

#include <string>

class Address {
public:
  Address() = default;
  Address(const std::string &ip, int port) : ip_(ip), port_(port) {}

  inline bool operator==(const Address &other) const {
    return ip_ == other.ip() && port_ == other.port();
  }
  inline bool operator!=(const Address &other) const {
    return !(*this == other);
  }

  const std::string addr() const { return ip_ + ":" + std::to_string(port_); }
  const std::string &ip() const { return ip_; }
  const int port() const { return port_; }

private:
  std::string ip_;
  int port_;
};

class Server {
public:
  Server() = default;
  Server(const Address &fwd_addr, const Address &bind_addr)
      : fwd_addr_(fwd_addr), bind_addr_(bind_addr) {}

  const Address &fwd_addr() const { return fwd_addr_; }
  const Address &bind_addr() const { return bind_addr_; }

private:
  Address fwd_addr_;
  Address bind_addr_;
};

#endif // ADDRPORT_HPP
