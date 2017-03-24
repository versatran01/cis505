#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "addrport.hpp"
#include <string>

class Client {
public:
  explicit Client(const AddrPort &addrport)
      : addrport_(addrport), nick_(addrport.full()), room_(-1) {}
  Client(const AddrPort &addrport, int room)
      : addrport_(addrport), nick_(addrport.full()), room_(room) {}

  const AddrPort addrport() const { return addrport_; }
  void set_nick(const std::string &nick) { nick_ = nick; }
  const std::string &nick() const { return nick_; }
  const std::string nick(char delim) const { return delim + nick_ + delim; }
  int room() const { return room_; }
  void set_room(int room) { room_ = room; }
  bool InRoom() const { return room_ > 0; }

private:
  AddrPort addrport_;
  std::string nick_;
  int room_;
};

#endif // CLIENT_HPP
