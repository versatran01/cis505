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

  std::string addr() const { return ip_ + ":" + std::to_string(port_); }
  const std::string &ip() const { return ip_; }
  const int port() const { return port_; }

private:
  std::string ip_;
  int port_;
};

#endif // ADDRPORT_HPP
