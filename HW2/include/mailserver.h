#ifndef MAILSERVER_H
#define MAILSERVER_H

#include "server.h"
#include "user.h"

class MailServer : public Server {
public:
  MailServer(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  const std::string &mailbox() const { return mailbox_; }
  const std::vector<UserPtr> &users() const { return users_; }

  void LoadMailbox();
  bool UserExistsByMailAddr(const std::string &mail_addr) const;
  bool UserExistsByUsername(const std::string &username) const;
  UserPtr GetUserByMailAddr(const std::string &mail_addr) const;
  UserPtr GetUserByUsername(const std::string &username) const;

protected:
  std::string mailbox_;
  std::vector<UserPtr> users_;
};

#endif // MAILSERVER_H
