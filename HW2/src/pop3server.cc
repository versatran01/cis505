#include "pop3server.h"
#include "lpi.h"
#include "mail.h"

#include "fsm.hpp"
#include "loguru.hpp"

#include <algorithm>
#include <openssl/md5.h>

enum class State { Init, User, Pass, Trans, Update };
enum class Trigger { CONN_, USER, PASS_OK, PASS_ERR, QUIT, STAT };
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
  auto reset_user = [&]() { user_.reset(); };
  auto reply_err = [&](const std::string &msg) { ReplyErr(fd, msg); };
  auto reply_ok = [&](const std::string &msg) { ReplyOk(fd, msg); };

  Pop3Fsm fsm;
  // from, to, trigger, guard, action
  fsm.add_transitions({
      {State::Init, State::User, Trigger::CONN_, nullptr, greet},
      {State::User, State::Pass, Trigger::USER, nullptr, nullptr},
      {State::Pass, State::User, Trigger::PASS_ERR, nullptr, reset_user},
      {State::Pass, State::Trans, Trigger::PASS_OK, nullptr, nullptr},
  });

  fsm.execute(Trigger::CONN_);
  CHECK_F(fsm.state() == State::User, "Init -- CONN/greet --> Auth");

  while (true) {
    std::string request;
    ReadLine(fd, request);

    // Extract command, for now assume no preceeding white spaces
    const auto command = ExtractCommand(request);
    LOG_F(INFO, "[%d] cmd={%s}", fd, command.c_str());

    if (command == "USER") { // ===== USER =====
      if (fsm.state() != State::User) {
        reply_err("USER only works in AUTORHIZATION");
        continue;
      }

      const auto username = ExtractArguments(request);
      user_ = GetUserByUsername(username);
      if (!user_) {
        reply_err("No mailbox here for " + username);
        LOG_F(WARNING, "No mailbox here for, user={%s}", username.c_str());
        continue;
      }

      fsm.execute(Trigger::USER);
      reply_ok(username + " is a valid mailbox");

      CHECK_F(fsm.state() == State::Pass, "Auth_Name -- USER --> Auth_Pass");
    } else if (command == "PASS") { // ===== PASS =====
      if (fsm.state() != State::Pass) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }

      // Check password
      const auto password = ExtractArguments(request);
      if (user_->password() == password) {
        if (user_->mutex()->try_lock()) {

          fsm.execute(Trigger::PASS_OK);

          reply_ok("maildrop locked and ready");
          LOG_F(INFO, "[%d] correct passwrod", fd);

          // Prepare maildrop
          maildrop_ = user_->ReadMaildrop();
          LOG_F(INFO, "[%d] Read maildrop, n={%zu}", fd, maildrop_.NumMails());

          CHECK_F(fsm.state() == State::Trans,
                  "Auth_Pass -- PASS_OK --> Trans");
        } else {
          reply_err("unable to lock maildrop");
          fsm.execute(Trigger::PASS_ERR);

          LOG_F(WARNING, "[%d] Maildrop already locked", fd);
          CHECK_F(fsm.state() == State::User,
                  "Auth_Pass -- PASS_ERR --> Auth_User");
        }
      } else {
        reply_err("invalid password");
        fsm.execute(Trigger::PASS_ERR);

        LOG_F(WARNING, "[%d] Invalid password", fd);
        CHECK_F(fsm.state() == State::User,
                "Auth_Pass -- PASS_ERR --> Auth_User");
      }
    } else if (command == "STAT") { // ===== STAT =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }

      const auto n = maildrop_.NumMails();
      const auto octets = maildrop_.TotalOctets();
      const auto msg = std::to_string(n) + " " + std::to_string(octets);
      reply_ok(msg);
      LOG_F(INFO, "[%d] Stat, n={%zu}, octets={%zu}", fd, n, octets);

    } else if (command == "LIST") { // ====== LIST =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }
    } else if (command == "RETR") { // ===== RETR =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }
    } else if (command == "DELE") { // ===== DELE =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }
    } else if (command == "NOOP") { // ===== NOOP =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }
    } else if (command == "UIDL") { // ===== UIDL =====
      if (fsm.state() != State::Trans) {
        reply_err("PASS only works in AUTORHIZATION");
        continue;
      }
    } else if (command == "QUIT") { // ===== QUIT =====
      if (fsm.state() == State::Trans) {
        // Update maildrop

      } else if (fsm.state() == State::User || fsm.state() == State::User) {
        const auto msg = "POP3 server signing off";
        reply_ok(msg);
        LOG_F(INFO, "[%d] %s", fd, msg);
      }

      // Close socket and mark it as closed
      close(fd);
      LOG_F(INFO, "[%d] Connection closed", fd);
      if (verbose_)
        fprintf(stderr, "[%d] Connection closed\n", fd);

      // Set socket fd to -1
      std::lock_guard<std::mutex> guard(open_sockects_mutex_);
      *sock_ptr = -1;
      return;
    } else {
      ReplyErr(fd, "Unknown command");
    }
  }
}

void Pop3Server::ReplyOk(int fd, const std::string &message) const {
  WriteLine(fd, "+OK " + message);
}

void Pop3Server::ReplyErr(int fd, const std::string &message) const {
  WriteLine(fd, "-ERR " + message);
}
