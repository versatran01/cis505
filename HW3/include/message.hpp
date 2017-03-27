#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "loguru.hpp"
#include <algorithm>
#include <string>
#include <vector>

enum MsgType { NORMAL, PROPOSE, DELIVER };
enum DeliverStatus { NOTDELIVERABLE, DELIVERBALE, DELIVERED };

struct Message {
  Message() = default;

  std::string Full() const { return "<" + nick + "> " + text; }
  size_t NumProposed() const { return proposed.size(); }
  int MaxProposed() const {
    auto it = std::max_element(proposed.begin(), proposed.end());
    CHECK_F(it != proposed.end());
    return *it;
  }

  inline bool operator<(const Message &other) const {
    if (seq == other.seq) {
      if (addr == other.addr) {
        return id < other.id;
      } else {
        return addr < other.addr;
      }
    } else {
      return seq < other.seq;
    }
  }

  int id;           // id from a client
  int seq;          // sequence number
  int room;         // group
  std::string addr; // full address
  std::string nick; // nickname
  std::string text; // message text
  DeliverStatus status = DeliverStatus::NOTDELIVERABLE;
  std::vector<int> proposed;
};

#endif // MESSAGE_HPP
