#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "address.hpp"
#include <string>

/**
 * @brief The Client class
 */
class Client {
public:
  explicit Client(const Address &addr)
      : addr_(addr), nick_(addr.full()), room_(-1) {}
  Client(const Address &addr, int room)
      : addr_(addr), nick_(addr.full()), room_(room) {}
  Client(const Address &addr, const std::string &nick)
      : addr_(addr), nick_(nick), room_(-1) {}

  const Address addr() const { return addr_; }

  void set_nick(const std::string &nick) { nick_ = nick; }
  const std::string &nick() const { return nick_; }
  const std::string nick2() const { return "<" + nick_ + "> "; }

  int room() const { return room_; }
  const std::string room_str() const { return std::to_string(room_); }

  bool InRoom() const { return room_ > 0; }
  void Join(int room) { room_ = room; }
  int Leave() {
    int old_room = room_;
    room_ = -1;
    return old_room;
  }

private:
  Address addr_;
  std::string nick_;
  int room_;
};

#endif // CLIENT_HPP
