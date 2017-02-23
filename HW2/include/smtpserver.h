#ifndef SMTP_SERVER_H
#define SMTP_SERVER_H

#include "mailserver.h"

#include <regex>

class SmtpServer : public MailServer {
public:
  SmtpServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;

  void ReplyCode(int fd, int code) const;
  void SendMail(const Mail &mail, int fd) const;

private:
  std::regex mail_from_regex_;
  std::regex rcpt_to_regex_;
  std::regex helo_regex_;
};

#endif // SMTP_SERVER_H
