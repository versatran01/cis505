#ifndef SMTP_SERVER_H
#define SMTP_SERVER_H

#include "server.h"
#include "user.h"

class SmtpServer : public Server {
 public:
  SmtpServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(const SocketPtr &sock_ptr) override;
  virtual void Stop() override;

  void Mailbox();

 private:
  std::string mailbox_;
  std::vector<User> users_;
};

#endif  // SMTP_SERVER_H
