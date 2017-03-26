#ifndef SERVER_HPP
#define SERVER_HPP

#include "address.hpp"

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

#endif // SERVER_HPP
