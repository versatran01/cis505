#include "totalorder.hpp"
#include "loguru.hpp"

#include <algorithm>

void TotalOrder::AddMsg(const Message &msg) {
  auto &q = hbq_[msg.room];
  q.push_back(msg);
}

int TotalOrder::NewProposed(int room) {
  auto Pg_old = Pg_[room];
  auto Ag = Ag_[room];
  auto Pg_new = std::max(Pg_old, Ag) + 1;
  ++Pg_[room];
  return Pg_new;
}

Message &TotalOrder::GetMsg(const std::string &addr, int room, int id) {
  auto cmp = [&](const Message &m) {
    return m.addr == addr && m.room == room && m.id == id;
  };

  auto &vec = hbq_[room];

  auto it = std::find_if(vec.begin(), vec.end(), cmp);
  CHECK_F(it != vec.end());

  return *it;
}

void TotalOrder::UpdateAgreed(int room, int seq) {
  auto Ag_old = Ag_[room];
  Ag_[room] = std::max(Ag_old, seq);
}

void TotalOrder::SortQueue(int room) {
  auto &q = hbq_[room];
  std::sort(q.begin(), q.end());
}

void TotalOrder::RemoveDelivered(int room) {
  auto &q = hbq_[room];
  auto cmp = [](const Message &m) {
    return m.status == DeliverStatus::DELIVERED;
  };
  auto beg = std::remove_if(q.begin(), q.end(), cmp);
  q.erase(beg, q.end());
}
