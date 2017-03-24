#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "addrport.hpp"
#include <string>

class Client {
public:
  const AddrPort addrport() const { return addrport_; }
  const std::string &nick() const { return nick_; }
  int room() const { return room_; }

private:
  AddrPort addrport_;
  std::string nick_;
  int room_;
};

#endif // CLIENT_HPP
