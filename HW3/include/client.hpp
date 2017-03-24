#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "addrport.hpp"
#include <string>

class Client {
public:
  explicit Client(const AddrPort &addr)
      : addr_(addr), nick_(addr.full()), room_(-1) {}
  Client(const AddrPort &addr, int room)
      : addr_(addr), nick_(addr.full()), room_(room) {}
  Client(const AddrPort &addr, const std::string &nick)
      : addr_(addr), nick_(nick), room_(-1) {}

  const AddrPort addr() const { return addr_; }

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
  AddrPort addr_;
  std::string nick_;
  int room_;
};

#endif // CLIENT_HPP
