#include "fsm.hpp"
#include <cassert>
#include <iostream>

enum class State { Init, Ready, Wait, Mail, Rcpt };
enum class Trigger {
  CONN, // On connection
  HELO,
  MAIL,
  RCPT,
  RSET,
  NOOP,
  QUIT,
  DATA,
};
using MyFsm = FSM::Fsm<State, State::Init, Trigger>;

int main() {
  auto greet = []() { std::cout << "Service Ready" << std::endl; };
  auto ok = []() { std::cout << "OK" << std::endl; };

  MyFsm fsm;

  // from, to, trigger, guard, action
  fsm.add_transitions({
      {State::Init, State::Ready, Trigger::CONN, nullptr, greet},
      {State::Ready, State::Wait, Trigger::HELO, nullptr, ok},
      {State::Wait, State::Mail, Trigger::MAIL, nullptr, ok},
  });

  fsm.execute(Trigger::CONN);
  assert(fsm.state() == State::Ready);

  fsm.execute(Trigger::MAIL);
  assert(fsm.state() == State::Ready);

  fsm.execute(Trigger::HELO);
  assert(fsm.state() == State::Wait);

  fsm.execute(Trigger::MAIL);
  assert(fsm.state() == State::Mail);
}
