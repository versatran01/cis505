#include "client.hpp"
#include "loguru.hpp"

int Client::LeaveRoom() {
  int old_room = room_;
  room_ = -1;
  return old_room;
}

int Client::IncSeq() {
  CHECK_F(room_ > 0, "Has to be a valid room");
  int seq = seqs_[room_]; // will be 0 if this room doesn't exist previouslly
  seqs_[room_]++;
  return seq;
}
