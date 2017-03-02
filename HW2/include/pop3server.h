#ifndef POP3SERVER_H
#define POP3SERVER_H

#include "maildrop.h"
#include "mailserver.h"

#include <functional>
#include <regex>

using CmdHandle = std::function<void(int, const Maildrop &, int)>;

class Pop3Server : public MailServer {
public:
  Pop3Server(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;
  void ReplyOk(int fd, const std::string &msg) const;
  void ReplyErr(int fd, const std::string &msg) const;

private:
  bool User(int fd, UserPtr &user, const std::string &req) const;
  void Pass(int fd) const;
  void Send(int fd, const Mail &mail) const;
  void Stat(int fd, const Maildrop &maildrop) const;
  void List(int fd, const Maildrop &md, int arg) const;
  void Uidl(int fd, const Maildrop &md, const std::string &req) const;
  void Retr(int fd, const Maildrop &md, int arg) const;
  void Dele(int fd, const Maildrop &md, int arg) const;

  void IntArgCmd(int fd, const Maildrop &md, const std::string &req,
                 const std::string &cmd, CmdHandle &handle) const;
  void OptIntArgCmd(int fd, const Maildrop &md, const std::string &req,
                    const std::string &cmd, CmdHandle &handle) const;
};

#endif // POP3SERVER_H
