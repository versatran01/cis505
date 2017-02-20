#ifndef POP3SERVER_H
#define POP3SERVER_H

#include "server.h"
#include "user.h"

class Pop3Server : public Server {
public:
  Pop3Server(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;

  void Mailbox();
  bool UserExistsByAddr(const std::string &mail_addr) const;

private:
  std::string mailbox_; // name of mailbox
  std::vector<User> users_;
}

#endif // POP3SERVER_H
