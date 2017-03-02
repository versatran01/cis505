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
  bool User(int fd, UserPtr &user, const std::string &request) const;
  void Pass(int fd) const;
  void Send(int fd, const Mail &mail) const;
  void Stat(int fd, const Maildrop &maildrop) const;
  void List(int fd, const Maildrop &maildrop, const std::string &request) const;
  void Retr(int fd, const Maildrop &maildrop, const std::string &request) const;
  void Dele(int fd, const Maildrop &maildrop, const std::string &request) const;
  void Uidl(int fd, const Maildrop &maildrop, const std::string &request) const;
};

#endif // POP3SERVER_H
