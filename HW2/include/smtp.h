#ifndef SMTP_H
#define SMTP_H

#include "server.h"

class SmtpServer : public Server {
public:
  SmtpServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(const SocketPtr &sock_ptr) override;
  virtual void Stop() override;

  void Mailbox();

private:
  std::string mailbox_;
  std::vector<std::string> users_;
};

#endif // SMTP_H
