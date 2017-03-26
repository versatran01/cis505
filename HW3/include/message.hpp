#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

struct Message {
  Message() = default;

  std::string Full() const { return "<" + nick + "> " + text; }
  size_t NumProposed() const { return proposed.size(); }

  int seq;
  int room;
  std::string addr;
  std::string nick;
  std::string text;
  bool deliverble = false;
  std::vector<int> proposed;
};

#endif // MESSAGE_HPP
