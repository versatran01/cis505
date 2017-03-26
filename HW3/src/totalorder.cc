#include "totalorder.hpp"

int TotalOrder::NewProposed(int room) {
  int Pg_old = Pg_[room];
  int Ag = Ag_[room];
  Pg_new = std::max(Pg_old, Ag) + 1;
  ++Pg_[room];
  return Pg_new;
}
