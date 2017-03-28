#ifndef TOTALORDER_HPP
#define TOTALORDER_HPP

#include "message.hpp"
#include <map>
#include <vector>

class TotalOrder {
public:
  int NewProposed(int room);

  void AddMsg(const Message &msg);

  Message &GetMsg(const std::string &addr, int room, int id);

  size_t QueueSize(int room) { return hbq_[room].size(); }

  void UpdateAgreed(int room, int seq);

  void SortQueue(int room);

  std::vector<Message> &GetQueue(int room) { return hbq_[room]; }

  void RemoveDelivered(int room);

private:
  // Highest sequence number it has propsed to group g so far
  std::map<int, int> Pg_;
  // Highest agreed sequence number it has seen for group g
  std::map<int, int> Ag_;
  // local hold back queue
  std::map<int, std::vector<Message>> hbq_;
};

#endif // TOTALORDER_HPP
