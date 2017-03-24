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
  Client(const AddrPort &addrport, const std::string &nick)
      : addrport_(addrport), nick_(nick), room_(-1) {}

  const AddrPort addrport() const { return addrport_; }

  void set_nick(const std::string &nick) { nick_ = nick; }
  const std::string &nick() const { return nick_; }
  const std::string nick(char delim) const { return delim + nick_ + delim; }

  int room() const { return room_; }
  void set_room(int room) { room_ = room; }
  const std::string room_str() const { return std::to_string(room_); }
  bool InRoom() const { return room_ > 0; }
  int leave() {
    int old_room = room_;
    room_ = -1;
    return old_room;
  }

private:
  AddrPort addrport_;
  std::string nick_;
  int room_;
};

#endif // CLIENT_HPP
