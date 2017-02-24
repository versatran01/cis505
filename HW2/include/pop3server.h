#ifndef POP3SERVER_H
#define POP3SERVER_H

#include "maildrop.h"
#include "mailserver.h"

#include <regex>

class Pop3Server : public MailServer {
public:
  Pop3Server(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;
  void ReplyOk(int fd, const std::string &message) const;
  void ReplyErr(int fd, const std::string &message) const;

private:
  UserPtr user_;
  MailDrop mail_drop_;
};

#endif // POP3SERVER_H
