#include "holdbackqueue.hpp"
#include <algorithm>

void HoldbackQueueFIFO::AddMsg(const Message &msg) { queue_.push_back(msg); }

int HoldbackQueueFIFO::GetMsgIndex(const std::string &addr, int room,
                                   int seq) const {
  auto cmp = [&](const Message &msg) {
    return msg.addr == addr && msg.room == room && msg.seq == seq;
  };
  auto it = std::find_if(queue_.begin(), queue_.end(), cmp);
  if (it == queue_.end()) {
    return -1;
  } else {
    return std::distance(queue_.begin(), it);
  }
}

void HoldbackQueueFIFO::RemoveByIndex(int index) {
  queue_.erase(queue_.begin() + index);
}
