#include "smtpserver.h"
#include "fsm.hpp"
#include "lpi.h"
#include "string_algorithms.h"

#include "loguru.hpp"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

enum class State { Init, Ready, Wait };
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
using SmtpFsm = FSM::Fsm<State, State::Init, Trigger>;

SmtpServer::SmtpServer(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : Server(port_no, backlog, verbose), mailbox_(mailbox) {
  if (mailbox_.empty()) {
    LOG_F(ERROR, "No mailbox, dir={%s}", mailbox_.c_str());
  } else {
    LOG_F(INFO, "Mailbox, dir={%s}", mailbox_.c_str());
  }
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

  // Upon connection, send service ready
  //  WriteLine(fd, "220 localhost service ready");
  auto greet = [&fd]() { WriteLine(fd, "220 localhost service ready"); };

  SmtpFsm fsm;
  fsm.add_transitions({
      // from, to, trigger, guard, action
      {State::Init, State::Ready, Trigger::CONN, nullptr, greet},
  });

  fsm.execute(Trigger::CONN);

  // State machine here
  while (true) {
    std::string request;
    ReadLine(fd, request);
    trim(request);

    // DEBUG_PRINT
    if (verbose_)
      fprintf(stderr, "[%d] C: %s\n", fd, request.c_str());
    LOG_F(INFO, "Read from fd={%d}, str={%s}", fd, request.c_str());

    // Extract command
    auto command = ExtractCommand(request);

    // Check if it is ECHO or QUIT
    if (command == "ECHO") {
      auto text = request.substr(5);
      trim_front(text);
      auto response = std::string("+OK ") + text;
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={ECHO}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
    } else if (command == "QUIT") {
      auto response = std::string("+OK Goodbye!");
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={QUIT}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
      if (verbose_)
        fprintf(stderr, "[%d] Connection closed\n", fd);

      // Close socket and mark it as closed
      close(fd);
      *sock_ptr = -1;
      return;
    } else {
      auto response = std::string("-ERR Unknown command");
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={UNKNOWN}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
    }
  }
}

void SmtpServer::Stop() {}
