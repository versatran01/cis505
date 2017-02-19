#ifndef SMTP_SERVER_H
#define SMTP_SERVER_H

#include "mail.h"
#include "server.h"
#include "user.h"

#include <regex>

class SmtpServer : public Server {
public:
  SmtpServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;

  void Mailbox();
  bool UserExists(const std::string &mail_addr) const;

private:
  std::string mailbox_; // name of mailbox
  std::vector<User> users_;

  std::regex mail_from_regex_;
  std::regex rcpt_to_regex_;
  std::regex helo_regex_;
};

#endif // SMTP_SERVER_H
