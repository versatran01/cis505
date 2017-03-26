#ifndef TOTALORDER_HPP
#define TOTALORDER_HPP

#include "message.hpp"
#include <map>
#include <vector>

class TotalOrder {
public:
  int NewProposed(int room) {
    int Pg_old = Pg_[room];
    int Ag = Ag_[room];
    Pg_new = std::max(Pg_old, Ag) + 1;
    ++Pg_[room];
    return Pg_new;
  }

private:
  // Highest sequence number it has propsed to group g so far
  std::map<int, int> Pg_;
  // Highest agreed sequence number it has seen for group g
  std::map<int, int> Ag_;
  // (m, Pg_new) local hold back queue
  std::vector<Message> hbq_;
};

#endif // TOTALORDER_HPP
