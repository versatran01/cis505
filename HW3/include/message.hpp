#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

struct Message {
  Message() = default;

  std::string FullMsg() const { return "<" + nick + "> " + msg; }

  int seq;
  int room;
  std::string addr;
  std::string nick;
  std::string msg;
};

#endif // MESSAGE_HPP
