#include "smtpserver.h"
#include "lpi.h"
#include "mail.h"
#include "string_algorithms.h"

#include "fsm.hpp"
#include "loguru.hpp"

#include <algorithm>

enum class State { Init, Idle, Wait, Mail, Rcpt, Data, Send };
enum class Trigger { HELO, MAIL, RCPT, RSET, QUIT, DATA, CONN_, EOML_, SENT_ };
using SmtpFsm = FSM::Fsm<State, State::Init, Trigger>; // State machine

void SmtpServer::ReplyCode(int fd, int code) const {
  if (code == 503) {
    const auto reply = "503 Bad sequence of commands";
    WriteLine(fd, reply);
  } else if (code == 501) {
    const auto reply = "501 Syntax error in parameters or arguments";
    WriteLine(fd, reply);
  } else {
    LOG_F(WARNING, "[%d] Unknown code, code={%d}", fd, code);
  }
}

SmtpServer::SmtpServer(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : MailServer(port_no, backlog, verbose, mailbox) {
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

void SmtpServer::Work(SocketPtr sock_ptr) {
  const auto fd = *sock_ptr;
  LOG_F(INFO, "[%d] Inside SmtpServer::Work", fd);

  Mail mail;
  // Actions
  auto greet = [&]() { WriteLine(fd, "220 localhost service ready"); };
  auto ok = [&]() { WriteLine(fd, "250 OK"); };
  auto ok_helo = [&]() { WriteLine(fd, "250 localhost"); };
  auto reset = [&]() { mail.Clear(); };
  auto ok_reset = [&]() {
    reset();
    ok();
  };
  auto ok_data = [&]() { WriteLine(fd, "354 Start mail input"); };

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

    // Text state
    if (fsm.state() == State::Data) {
      if (request == ".") {
        // End of mail
        mail.Stamp(); // Time stamp mail

        fsm.execute(Trigger::EOML_);

        const auto msg = "State transition: Data -- ./ok --> Send";
        CHECK_F(fsm.state() == State::Send);
        LOG_F(INFO, "[%d] %s", fd, msg);
        continue;
      }

      mail.AddLine(request);
      continue;
    }

    // Send mail to mbox
    if (fsm.state() == State::Send) {
      SendMail(mail, fd);

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
        ReplyCode(fd, 503);
        continue;
      }

      // Try match "HELO <domain>"
      std::smatch results;
      if (!std::regex_search(request, results, helo_regex_)) {
        LOG_F(WARNING, "[%d] Match HELO failed", fd);
        ReplyCode(fd, 501);
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
        ReplyCode(fd, 503);
        continue;
      }

      // Try match "MAIL FROM:<some.guy@somewhere>"
      std::smatch results;
      if (!std::regex_search(request, results, mail_from_regex_)) {
        LOG_F(WARNING, "[%d] Match MAIL FROM failed", fd);
        ReplyCode(fd, 501);
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
        ReplyCode(fd, 503);
        continue;
      }

      // Try match "RCPT TO:<some.guy@somewhere>"
      std::smatch results;
      if (!std::regex_search(request, results, rcpt_to_regex_)) {
        LOG_F(WARNING, "[%d] Match RCPT TO failed", fd);
        ReplyCode(fd, 501);
        continue;
      }

      const auto &mail_addr = results.str(1);
      LOG_F(INFO, "[%d] Valid RCPT TO, mail_addr={%s}", fd, mail_addr.c_str());

      // Check if user exists
      if (!UserExistsByAddr(mail_addr)) {
        LOG_F(INFO, "[%d] User doesn't exist, mail_addr={%s}", fd,
              mail_addr.c_str());
        WriteLine(fd, "550 No such user");
        continue;
      }

      if (!mail.RecipientExists(mail_addr)) {
        mail.AddRecipient(mail_addr);
        LOG_F(INFO, "[%d] Recipient added, mail_addr={%s}", fd,
              mail_addr.c_str());
      } else {
        LOG_F(WARNING, "[%d] Recipient already added, mail_addr={%s}", fd,
              mail_addr.c_str());
      }

      fsm.execute(Trigger::RCPT);

      LOG_F(INFO, "[%d] Number of recipients, n={%zu}", fd,
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
        ReplyCode(fd, 503);
        continue;
      }

      fsm.execute(Trigger::RSET);

      const auto msg = "State transition: Mail/Rcpt -- RSET/reset --> Wait";
      CHECK_F(fsm.state() == State::Wait);
      CHECK_F(mail.Empty());
      LOG_F(INFO, "[%d] %s", fd, msg);
    } else if (command == "NOOP") {
      // ===== NOOP =====
      LOG_F(INFO, "[%d] NOOP", fd);
      WriteLine(fd, "250 OK");
    } else if (command == "DATA") {
      // ===== DATA =====
      if (fsm.state() != State::Rcpt) {
        ReplyCode(fd, 503);
      }

      fsm.execute(Trigger::DATA);

      const auto msg = "State transition: Rcpt -- Data/ok_data --> Data";
      CHECK_F(fsm.state() == State::Data);
      LOG_F(INFO, "[%d] %s", fd, msg);
    } else if (command == "QUIT") {
      WriteLine(fd, "221 localhost Service closing");

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
      WriteLine(fd, "500 Syntax error, command unrecognized");
    }
  }
}

void SmtpServer::SendMail(const Mail &mail, int fd) const {
  for (const auto &recipient : mail.recipients()) {
    auto user_iter = std::find_if(
        users_.begin(), users_.end(),
        [&recipient](const User &user) { return user.mail_addr() == recipient; });

    // This should never happen, because we checked this in Rcpt state
    if (user_iter == users_.end()) {
      LOG_F(ERROR, "[%d] Could not find recipient, mail_addr={%s}", fd,
            recipient.c_str());
      continue;
    }

    user_iter->WriteMail(mail);
    LOG_F(INFO, "[%d] Finish sending to recipient, mail_addr={%s}", fd,
          recipient.c_str());
  }
}
