#ifndef MAILSERVER_H
#define MAILSERVER_H

#include "server.h"
#include "user.h"

class MailServer : public Server {
public:
  MailServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  const std::string &mailbox() const { return mailbox_; }
  const std::vector<User> &users() const { return users_; }

  void LoadMailbox();
  bool UserExistsByAddr(const std::string &mail_addr) const;

protected:
  std::string mailbox_;
  std::vector<User> users_;
};

#endif // MAILSERVER_H
