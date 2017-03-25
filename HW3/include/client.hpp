#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "address.hpp"
#include <map>
#include <string>

/**
 * @brief The Client class
 */
class Client {
public:
  Client() = default;
  explicit Client(const Address &addr)
      : addr_(addr), nick_(addr.addr()), room_(0) {}
  Client(const Address &addr, int room)
      : addr_(addr), nick_(addr.addr()), room_(room) {}
  Client(const Address &addr, const std::string &nick)
      : addr_(addr), nick_(nick), room_(-1) {}

  inline bool operator==(const Client &other) const {
    return addr_ == other.addr();
  }
  inline bool operator!=(const Client &other) const {
    return !(*this == other);
  }

  const Address &addr() const { return addr_; }
  const std::string addr_str() const { return addr_.addr(); }

  void set_nick(const std::string &nick) { nick_ = nick; }
  const std::string &nick() const { return nick_; }
  const std::string nick_prefix() const { return "<" + nick_ + "> "; }

  int room() const { return room_; }
  const std::string room_str() const { return std::to_string(room_); }

  bool InRoom() const { return room_ > 0; }
  bool InRoom(int room) const { return room_ == room; }

  void JoinRoom(int room) { room_ = room; }
  int LeaveRoom();

  std::map<int, int> &seqs() { return seqs_; }
  int IncSeq();

private:
  Address addr_;
  std::string nick_;
  int room_ = 0;
  std::map<int, int> seqs_;
};

#endif // CLIENT_HPP
