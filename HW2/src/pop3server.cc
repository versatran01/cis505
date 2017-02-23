#include "pop3server.h"
#include "mail.h"

#include "fsm.hpp"
#include "loguru.hpp"

#include <algorithm>
#include <openssl/md5.h>

enum class State { Init, Auth, Trans, Update };
enum class Trigger {
  CONN_,
};
using Pop3Fsm = FSM::Fsm<State, State::Init, Trigger>; // State machine

void computeDigest(char *data, int dataLengthBytes,
                   unsigned char *digestBuffer) {
  /* The digest will be written to digestBuffer, which must be at least
   * MD5_DIGEST_LENGTH bytes long */

  MD5_CTX c;
  MD5_Init(&c);
  MD5_Update(&c, data, dataLengthBytes);
  MD5_Final(digestBuffer, &c);
}

Pop3Server::Pop3Server(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : MailServer(port_no, backlog, verbose, mailbox) {}

void Pop3Server::Work(SocketPtr sock_ptr) {
  const auto fd = *sock_ptr;
  LOG_F(INFO, "[%d] Inside Pop3Server::Work", fd);

  // Actions
  auto greet = [&]() { WriteLine(fd, "+OK POP3 server ready"); };

  Pop3Fsm fsm;
  // from, to, trigger, guard, action
  fsm.add_transitions({
      {State::Init, State::Auth, Trigger::CONN_, nullptr, greet},
  });

  fsm.execute(Trigger::CONN_);
  CHECK_F(fsm.state() == State::Auth, "Init -- CONN/greet --> Auth");

  while (true) {
    std::string request;
    ReadLine(fd, request);

    // Extract command, for now assume no preceeding white spaces
    const auto command = ExtractCommand(request);
    LOG_F(INFO, "[%d] cmd={%s}", fd, command.c_str());

    if (fsm.state() == State::Auth) {
    } else if (fsm.state() == State::Trans) {
    } else if (fsm.state() == State::Update) {
    }
  }
}
