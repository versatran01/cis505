#ifndef HOLDBACKQUEUE_HPP
#define HOLDBACKQUEUE_HPP

#include "message.hpp"
#include <vector>

class HoldbackQueueFIFO {
public:
  void AddMsg(const Message &msg);

  int GetMsgIndex(const std::string &addr, int room, int seq) const;

  const Message &GetMsg(size_t index) const { return queue_[index]; }

  void RemoveByIndex(int index);

  size_t size() const { return queue_.size(); }

private:
  std::vector<Message> queue_;
};

#endif // HOLDBACKQUEUE_HPP
