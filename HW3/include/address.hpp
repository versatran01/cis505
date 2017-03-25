#ifndef ADDRPORT_HPP
#define ADDRPORT_HPP

#include <string>

class Address {
public:
  Address() = default;
  Address(const std::string &addr, int port) : addr_(addr), port_(port) {}

  inline bool operator==(const Address &other) const {
    return addr_ == other.addr() && port_ == other.port();
  }
  inline bool operator!=(const Address &other) const {
    return !(*this == other);
  }

  const std::string full_str() const {
    return addr_ + ":" + std::to_string(port_);
  }
  const std::string &addr() const { return addr_; }
  const int port() const { return port_; }

private:
  std::string addr_;
  int port_;
};

class Server {
public:
  Server() = default;
  Server(const Address &forward, const Address &binding)
      : forward_(forward), binding_(binding) {}

  const Address &forward() const { return forward_; }
  const Address &binding() const { return binding_; }

private:
  Address forward_;
  Address binding_;
};

#endif // ADDRPORT_HPP
