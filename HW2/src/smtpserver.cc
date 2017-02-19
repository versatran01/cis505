#include "smtpserver.h"
#include "fsm.hpp"
#include "lpi.h"
#include "string_algorithms.h"

#include "loguru.hpp"

#include <algorithm>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum class State { Init, Idle, Wait, Mail, Rcpt, Data, Send };
enum class Trigger { HELO, MAIL, RCPT, RSET, QUIT, DATA, CONN_, EOML_, SENT_ };
using SmtpFsm = FSM::Fsm<State, State::Init, Trigger>;

void SmtpReply(int fd, int code, const std::string &msg = "") {
  if (code == 503) {
    const auto reply = "503 Bad sequence of commands";
    WriteLine(fd, reply);
    LOG_F(WARNING, "[%d] %s", fd, reply);
  }
}

SmtpServer::SmtpServer(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : Server(port_no, backlog, verbose), mailbox_(mailbox) {
  if (mailbox_.empty()) {
    LOG_F(ERROR, "No mailbox, dir={%s}", mailbox_.c_str());
  } else {
    LOG_F(INFO, "Mailbox, dir={%s}", mailbox_.c_str());
  }

  // Construct regular expression for mail from and rcpt to and helo
  helo_regex_ = std::regex("HELO ([[:alnum:]]+)", std::regex::icase);

  // TODO: Need to fix this pattern
  std::string mail_addr_pattern("[[:alnum:]]+[[:alnum:].]*");
  mail_addr_pattern = mail_addr_pattern + "@" + mail_addr_pattern;

  std::string mail_from_pattern = "MAIL FROM:[ ]*<(" + mail_addr_pattern + ")>";
  mail_from_regex_ = std::regex(mail_from_pattern, std::regex::icase);

  std::string rcpt_to_pattern = "RCPT TO:[ ]*<(" + mail_addr_pattern + ")>";
  rcpt_to_regex_ = std::regex(rcpt_to_pattern, std::regex::icase);
}

void SmtpServer::Mailbox() {
  // Current path is
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "Current path, dir={%s}", cwd.c_str());

  // Mailbox dir is
  fs::path mailbox_dir = cwd / mailbox_;

  // Check if it is a directory
  if (fs::is_directory(mailbox_dir)) {
    LOG_F(INFO, "Mailbox dir, path={%s}", mailbox_dir.c_str());
    // Read all users
    for (const fs::directory_entry &file :
         fs::directory_iterator(mailbox_dir)) {
      if (file.path().extension().string() == ".mbox") {
        const auto &name = file.path().stem().string();
        users_.emplace_back(name, file.path().string());
        LOG_F(INFO, "Add user, name={%s}", name.c_str());
        LOG_F(INFO, "User mbox, path={%s}", file.path().c_str());
      }
    }
  } else {
    const auto msg = "Mailbox dir invalid, path={%s}";
    LOG_F(ERROR, msg, mailbox_dir.c_str());
  }

  if (users_.empty()) {
    const auto msg = "No user found";
    LOG_F(ERROR, msg);
  }
  LOG_F(INFO, "Total user, n={%zu}", users_.size());
}

void SmtpServer::Work(const SocketPtr &sock_ptr) {
  LOG_F(INFO, "Inside SmtpServer::Work");
  const auto &fd = *sock_ptr;
  Mail mail;

  // Actions
  auto greet = [&fd]() { WriteLine(fd, "220 localhost service ready"); };
  auto ok = [&fd]() { WriteLine(fd, "250 OK"); };
  auto ok_helo = [&fd]() { WriteLine(fd, "250 localhost"); };
  auto reset = [&]() { mail.Clear(); };
  auto ok_reset = [&]() {
    mail.Clear();
    ok();
  };
  auto ok_data = [&fd]() { WriteLine(fd, "354 Start mail input"); };

  SmtpFsm fsm;
  // from, to, trigger, guard, action
  fsm.add_transitions({
      {State::Init, State::Idle, Trigger::CONN_, nullptr, greet},
      {State::Idle, State::Wait, Trigger::HELO, nullptr, ok_helo},
      {State::Wait, State::Mail, Trigger::MAIL, nullptr, ok},
      {State::Mail, State::Rcpt, Trigger::RCPT, nullptr, ok},
      {State::Rcpt, State::Rcpt, Trigger::RCPT, nullptr, ok},
      {State::Rcpt, State::Data, Trigger::DATA, nullptr, ok_data},
      {State::Data, State::Send, Trigger::EOML_, nullptr, ok},
      {State::Send, State::Wait, Trigger::SENT_, nullptr, reset},
      {State::Mail, State::Wait, Trigger::RSET, nullptr, ok_reset},
      {State::Rcpt, State::Wait, Trigger::RSET, nullptr, ok_reset},
  });

  // On connection
  fsm.execute(Trigger::CONN_);
  CHECK_F(fsm.state() == State::Idle, "Init -- CONN/greet --> Idle");

  // State machine here
  while (true) {
    std::string request;
    ReadLine(fd, request);

    // DEBUG_PRINT
    if (verbose_)
      fprintf(stderr, "[%d] C: %s\n", fd, request.c_str());
    LOG_F(INFO, "[%d] Read, str={%s}", fd, request.c_str());

    // Text state
    if (fsm.state() == State::Data) {
      if (request == ".") {
        // End of mail
        fsm.execute(Trigger::EOML_);
        const auto msg = "State transition: Data -- ./ok --> Send";
        CHECK_F(fsm.state() == State::Send);
        LOG_F(INFO, "[%d] %s", fd, msg);
        continue;
      }

      mail.AddLine(request);
      LOG_F(INFO, "[%d] Number of lines, n={%zu}", fd, mail.data().size());
      continue;
    }

    // Send mail to mbox
    if (fsm.state() == State::Send) {
      for (const auto &recipient : mail.recipients()) {
        auto user_iter = std::find_if(users_.begin(), users_.end(),
                                      [&recipient](const User &user) {
                                        return user.addr() == recipient;
                                      });
        // This should never happen, because we checked this in Rcpt state
        if (user_iter == users_.end()) {
          LOG_F(ERROR, "[%d] Could not find recipient, mail_addr={%s}", fd,
                recipient.c_str());
          continue;
        }

        const User &user = *user_iter;
        LOG_F(INFO, "[%d] Start sending to recipient, mail_addr={%s}", fd,
              recipient.c_str());

        SendMailToUser(mail, user);

        LOG_F(INFO, "[%d] Finish sending to recipient, mail_addr={%s}", fd,
              recipient.c_str());
      }

      LOG_F(INFO, "[%d] Finish sending mail to all recipients, n={zu}", fd,
            mail.recipients().size());

      fsm.execute(Trigger::SENT_);

      const auto msg = "State transition: Send -- SENT_/ --> Wait";
      CHECK_F(fsm.state() == State::Wait);
      LOG_F(INFO, "[%d] %s", fd, msg);
    }

    // Extract command, for now assume no preceeding white spaces
    auto command = ExtractCommand(request);
    LOG_F(INFO, "[%d] cmd={%s}", fd, command.c_str());

    // Check command
    if (command == "HELO") {
      // ===== HELO =====
      // State has to be Idle
      if (fsm.state() != State::Idle) {
        SmtpReply(fd, 503);
        continue;
      }

      // Try match "HELO <domain>"
      std::smatch results;
      if (!std::regex_search(request, results, helo_regex_)) {
        LOG_F(WARNING, "[%d] Match HELO failed", fd);
        continue;
      }

      const auto &domain = results.str(1);
      LOG_F(INFO, "[%d] Valid HELO, domain={%s}", fd, domain.c_str());

      fsm.execute(Trigger::HELO);

      const auto msg = "State transition: Idle -- HELO/ok --> Wait";
      CHECK_F(fsm.state() == State::Wait);
      LOG_F(INFO, "[%d] %s", fd, msg);

    } else if (command == "MAIL") {
      // ===== MAIL =====

      // State has to be Wait
      if (fsm.state() != State::Wait) {
        SmtpReply(fd, 503);
        continue;
      }

      // Try match "MAIL FROM:<some.guy@somewhere>"
      std::smatch results;
      if (!std::regex_search(request, results, mail_from_regex_)) {
        LOG_F(WARNING, "[%d] Match MAIL FROM failed", fd);
        continue;
      }

      const auto &mail_addr = results.str(1);
      LOG_F(INFO, "[%d] Valid MAIL FROM, mail_addr={%s}", fd,
            mail_addr.c_str());

      mail.set_sender(mail_addr);
      fsm.execute(Trigger::MAIL);

      const auto msg = "State transition: Wait -- MAIL/ok --> Mail";
      CHECK_F(fsm.state() == State::Mail);
      LOG_F(INFO, "[%d] %s", fd, msg);

    } else if (command == "RCPT") {
      // ===== RCPT =====

      // State has to be Mail or Rcpt
      if (!(fsm.state() == State::Mail || fsm.state() == State::Rcpt)) {
        SmtpReply(fd, 503);
        continue;
      }

      // Try match "RCPT TO:<some.guy@somewhere>"
      std::smatch results;
      if (!std::regex_search(request, results, rcpt_to_regex_)) {
        LOG_F(WARNING, "[%d] Match RCPT TO failed", fd);
        continue;
      }

      const auto &mail_addr = results.str(1);
      LOG_F(INFO, "[%d] Valid RCPT TO, mail_addr={%s}", fd, mail_addr.c_str());

      // Check if user exists
      if (!UserExists(mail_addr)) {
        const auto msg = "550 No such user";
        WriteLine(fd, msg);
        LOG_F(WARNING, "[%d] %s, mail_addr={%s}", fd, msg, mail_addr.c_str());
        continue;
      }

      if (!mail.RecipientExists(mail_addr)) {
        mail.AddRecipient(mail_addr);
        LOG_F(INFO, "[%d] Recipient added, mail_addr={%s}", fd,
              mail_addr.c_str());
      } else {
        LOG_F(WARNING, "[%d] Recipient already exists, mail_addr={%s}", fd,
              mail_addr.c_str());
      }

      fsm.execute(Trigger::RCPT);

      LOG_F(INFO, "[%d] Number of recipients, n={%zu}",
            mail.recipients().size());

      const auto msg = "State transition: Mail/Rcpt -- RCPT/ok --> Rcpt";
      CHECK_F(fsm.state() == State::Rcpt);
      LOG_F(INFO, "[%d] %s", fd, msg);

    } else if (command == "DATA") {
      // ===== DATA ====
      fsm.execute(Trigger::DATA);
    } else if (command == "RSET") {
      // ===== RSET =====
      if (!(fsm.state() == State::Mail || fsm.state() == State::Rcpt)) {
        SmtpReply(fd, 503);
        continue;
      }

      fsm.execute(Trigger::RSET);

      const auto msg = "State transition: Mail/Rcpt -- RSET/reset --> Wait";
      CHECK_F(fsm.state() == State::Wait);
      CHECK_F(mail.Empty());
      LOG_F(INFO, "[%d] %s", fd, msg);
    } else if (command == "NOOP") {
      LOG_F(INFO, "[%d] NOOP", fd);
    } else if (command == "DATA") {
      // ===== DATA =====
      if (fsm.state() != State::Rcpt) {
        SmtpReply(fd, 503);
      }

      fsm.execute(Trigger::DATA);

      const auto msg = "State transition: Rcpt -- Data/ok_data --> Data";
      CHECK_F(fsm.state() == State::Data);
      LOG_F(INFO, "[%d] %s", fd, msg);

    } else if (command == "QUIT") {
      const auto msg = "221 localhost Serice closing";
      WriteLine(fd, msg);

      // DEBUG_PRINT
      //      if (verbose_)
      //        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "[%d] Write to fd, str={%s}", fd, msg);
      //      if (verbose_)
      //        fprintf(stderr, "[%d] Connection closed\n", fd);

      // Close socket and mark it as closed
      close(fd);
      *sock_ptr = -1;
      return;
    } else {
      const auto msg = "500 Syntax error, command unrecognized";
      LOG_F(WARNING, "[%d] %s cmd={%s}", fd, msg, command);
      WriteLine(fd, msg);
    }
  }
}

void SmtpServer::Stop() {}

bool SmtpServer::UserExists(const std::string &mail_addr) const {
  auto pred = [&mail_addr](const User &user) {
    return user.addr() == mail_addr;
  };
  return std::find_if(users_.begin(), users_.end(), pred) != users_.end();
}

bool SmtpServer::SendMailToUser(const Mail &mail, const User &user) const {
  // Not implemented
  return true;
}
