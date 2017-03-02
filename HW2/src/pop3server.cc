#include "pop3server.h"
#include "lpi.h"
#include "mail.h"
#include "string_algorithms.h"

#include "fsm.hpp"
#include "loguru.hpp"

#include <algorithm>
#include <openssl/md5.h>

enum class State { Init, User, Pass, Trans, Update };
enum class Trigger { CONN_, USER, PASS_OK, PASS_ERR, QUIT, STAT };
using Pop3Fsm = FSM::Fsm<State, State::Init, Trigger>; // State machine

std::string UniqueId(const Mail &mail) {
  static const char *hex_digits = "0123456789ABCDEF";
  const auto data = mail.Data();
  std::vector<unsigned char> buffer(MD5_DIGEST_LENGTH);

  MD5_CTX c;
  MD5_Init(&c);
  MD5_Update(&c, data.data(), data.length());
  MD5_Final(buffer.data(), &c);

  std::string unique_id;
  unique_id.reserve(MD5_DIGEST_LENGTH * 2);
  for (size_t i = 0; i != MD5_DIGEST_LENGTH; ++i) {
    unique_id += hex_digits[buffer[i] / MD5_DIGEST_LENGTH];
    unique_id += hex_digits[buffer[i] % MD5_DIGEST_LENGTH];
  }
  return unique_id;
}

Pop3Server::Pop3Server(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : MailServer(port_no, backlog, verbose, mailbox) {}

void Pop3Server::Work(SocketPtr sock_ptr) {
  const auto fd = *sock_ptr;
  LOG_F(INFO, "[%d] Inside Pop3Server::Work", fd);

  Maildrop maildrop;
  UserPtr user;

  // Actions
  auto greet = [&]() { WriteLine(fd, "+OK POP3 server ready"); };
  auto reply_err = [&](const std::string &msg) { ReplyErr(fd, msg); };
  auto reply_ok = [&](const std::string &msg) { ReplyOk(fd, msg); };
  auto reset_user = [&]() { user.reset(); };

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
        ReplyErr(fd, "USER only works in AUTORHIZATION");
        continue;
      }

      if (User(fd, user, request)) {
        CHECK_F(user.get() != nullptr, "User is null");
        fsm.execute(Trigger::USER);
        CHECK_F(fsm.state() == State::Pass, "Auth_Name -- USER --> Auth_Pass");
      }
    } else if (command == "PASS") { // ===== PASS =====
      if (fsm.state() != State::Pass) {
        ReplyErr(fd, "PASS only works in AUTORHIZATION");
        continue;
      }

      // Check password
      const auto password = ExtractArgument(request);
      if (user->password() == password) {
        if (user->mutex()->try_lock()) {

          fsm.execute(Trigger::PASS_OK);

          reply_ok("maildrop locked and ready");
          LOG_F(WARNING, "[%d] correct passwrod", fd);

          // Prepare maildrop
          maildrop = user->ReadMaildrop();
          LOG_F(INFO, "[%d] Read maildrop, n={%zu}", fd, maildrop.NumMails());

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
        reply_err("STAT only works in TRANSACTION");
        continue;
      }

      Stat(fd, maildrop);
    } else if (command == "LIST") { // ====== LIST =====
      if (fsm.state() != State::Trans) {
        reply_err("LIST only works in TRANSACTION");
        continue;
      }

      List(fd, maildrop, request);
    } else if (command == "RETR") { // ===== RETR =====
      if (fsm.state() != State::Trans) {
        reply_err("RETR only works in TRANSACTION");
        continue;
      }

      Retr(fd, maildrop, request);
    } else if (command == "DELE") { // ===== DELE =====
      if (fsm.state() != State::Trans) {
        reply_err("DELE only works in TRANSACTION");
        continue;
      }

      Dele(fd, maildrop, request);
    } else if (command == "NOOP") { // ===== NOOP =====
      if (fsm.state() != State::Trans) {
        reply_err("NOOP only works in TRANSACTION");
        continue;
      }

      ReplyOk(fd, "");
    } else if (command == "UIDL") { // ===== UIDL =====
      if (fsm.state() != State::Trans) {
        reply_err("UIDL only works in TRANSACTION");
        continue;
      }

      Uidl(fd, maildrop, request);
    } else if (command == "QUIT") { // ===== QUIT =====
      if (fsm.state() == State::Trans) {
        // Update maildrop

      } else if (fsm.state() == State::User || fsm.state() == State::Pass) {
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
      std::lock_guard<std::mutex> guard(sockets_mutex_);
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

void Pop3Server::Stat(int fd, const Maildrop &maildrop) const {
  const auto n = maildrop.NumMails();
  const auto octets = maildrop.TotalOctets();
  const auto msg = std::to_string(n) + " " + std::to_string(octets);
  ReplyOk(fd, msg);
  LOG_F(INFO, "[%d] STAT, n={%zu}, octets={%zu}", fd, n, octets);
}

void Pop3Server::List(int fd, const Maildrop &maildrop,
                      const std::string &request) const {
  const auto n = maildrop.NumMails(false);
  const auto n_all = maildrop.NumMails(true);
  auto arg = ExtractArgument(request);
  trim(arg);

  if (arg.empty()) {
    LOG_F(INFO, "[%d] LIST no arg", fd);

    // Overall stat
    const auto octets = maildrop.TotalOctets();
    ReplyOk(fd, std::to_string(n) + " messages (" + std::to_string(octets) +
                    " octets)");

    // Output stat for each mail
    for (size_t i = 0; i < n_all; ++i) {
      const auto &mail = maildrop.GetMail(i);
      if (!mail.deleted()) {
        WriteLine(fd,
                  std::to_string(i + 1) + " " + std::to_string(mail.Octets()));
      }
    }
    WriteLine(fd, ".");
  } else {
    LOG_F(INFO, "[%d] LIST %s", fd, arg.c_str());

    const size_t i = std::stoi(arg);
    if (i > n_all || i <= 0) {
      ReplyErr(fd, "no such message, only " + std::to_string(n) +
                       " messages in maildrop");
    } else {
      const Mail &mail = maildrop.GetMail(i - 1);
      if (!mail.deleted()) {
        ReplyOk(fd, std::to_string(i) + " " + std::to_string(mail.Octets()));
      } else {
        ReplyErr(fd, "message already deleted");
      }
    }
  }
}

void Pop3Server::Retr(int fd, const Maildrop &maildrop,
                      const std::string &request) const {
  const auto n = maildrop.NumMails(false);
  const auto n_all = maildrop.NumMails(true);
  auto arg = ExtractArgument(request);
  trim(arg);

  if (arg.empty()) {
    ReplyErr(fd, "RETR needs one argument");
    return;
  }

  LOG_F(INFO, "[%d] RETR %s", fd, arg.c_str());

  const size_t i = std::stoi(arg);
  if (i > n_all || i <= 0) {
    ReplyErr(fd, "only " + std::to_string(n) + "/" + std::to_string(n_all) +
                     "messages");
  } else {
    const Mail &mail = maildrop.GetMail(i - 1);
    if (!mail.deleted()) {
      ReplyOk(fd, std::to_string(mail.Octets()) + " octets");
      Send(fd, mail);
    } else {
      ReplyErr(fd, "message already deleted");
    }
  }
}

bool Pop3Server::User(int fd, UserPtr &user, const std::string &request) const {
  const auto username = ExtractArgument(request);
  user = GetUserByUsername(username);

  if (!user) {
    ReplyErr(fd, "No mailbox here for " + username);
    LOG_F(WARNING, "No mailbox here for, user={%s}", username.c_str());
    return false;
  }

  ReplyOk(fd, username + " is a valid mailbox");
  LOG_F(WARNING, "Found valid mailbox, user={%s}", username.c_str());
  return true;
}

void Pop3Server::Send(int fd, const Mail &mail) const {
  for (const auto &line : mail.lines()) {
    WriteLine(fd, line);
  }
  WriteLine(fd, ".");
}

void Pop3Server::Dele(int fd, const Maildrop &maildrop,
                      const std::string &request) const {
  const auto n = maildrop.NumMails(false);
  const auto n_all = maildrop.NumMails(true);
  auto arg = ExtractArgument(request);
  trim(arg);

  if (arg.empty()) {
    ReplyErr(fd, "RETR needs one argument");
    return;
  }

  LOG_F(INFO, "[%d] DELE %s", fd, arg.c_str());

  const size_t i = std::stoi(arg);
  if (i > n_all || i <= 0) {
    ReplyErr(fd, "only " + std::to_string(n) + "/" + std::to_string(n_all) +
                     "messages");
  } else {
    const Mail &mail = maildrop.GetMail(i - 1);
    if (!mail.deleted()) {
      ReplyOk(fd, "message " + std::to_string(i) + " deleted");
      mail.MarkDeleted();
    } else {
      ReplyErr(fd, "message already deleted");
    }
  }
}

void Pop3Server::Uidl(int fd, const Maildrop &maildrop,
                      const std::string &request) const {
  const auto n = maildrop.NumMails(false);
  const auto n_all = maildrop.NumMails(true);
  auto arg = ExtractArgument(request);
  trim(arg);

  if (arg.empty()) {
    LOG_F(INFO, "[%d] UIDL no arg", fd);

    ReplyOk(fd, "");
    // Output stat for each mail
    for (size_t i = 0; i < n_all; ++i) {
      const auto &mail = maildrop.GetMail(i);
      if (!mail.deleted()) {
        WriteLine(fd, std::to_string(i + 1) + " " + UniqueId(mail));
      }
    }
    WriteLine(fd, ".");
  } else {
    LOG_F(INFO, "[%d] LIST %s", fd, arg.c_str());

    const size_t i = std::stoi(arg);
    if (i > n_all || i <= 0) {
      ReplyErr(fd, "no such message, only " + std::to_string(n) +
                       " messages in maildrop");
    } else {
      const Mail &mail = maildrop.GetMail(i - 1);
      if (!mail.deleted()) {
        ReplyOk(fd, std::to_string(i) + " " + UniqueId(mail));
      } else {
        ReplyErr(fd, "message already deleted");
      }
    }
  }
}
