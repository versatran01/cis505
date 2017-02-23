#ifndef USER_H
#define USER_H

#include <memory>
#include <mutex>
#include <string>

#include "mail.h"

class User {
public:
  using MutexPtr = std::shared_ptr<std::mutex>;

  User(const std::string &mailbox, const std::string &username);

  const std::string &mailbox() const { return mailbox_; }
  const std::string &username() const { return username_; }
  const std::string &password() const { return password_; }
  const std::string &mail_addr() const { return mail_addr_; }

  void WriteMail(const Mail &mail) const;

private:
  std::string mailbox_;
  std::string username_;
  std::string password_;
  std::string mail_addr_;
  MutexPtr mutex_;
};

#endif // USER_H
