#ifndef USER_H
#define USER_H

#include "mail.h"
#include "maildrop.h"

#include <memory>
#include <mutex>
#include <string>

using MutexPtr = std::shared_ptr<std::mutex>;

class User {
public:
  User(const std::string &mailbox, const std::string &username);

  // Getters
  const std::string &mailbox() const { return mailbox_; }
  const std::string &username() const { return username_; }
  const std::string &password() const { return password_; }
  const std::string &mailaddr() const { return mailaddr_; }
  const MutexPtr &mutex() const { return mutex_; }

  void WriteMail(const Mail &mail) const;
  Maildrop ReadMaildrop() const;
  void WriteMaildrop(const Maildrop &maildrop) const;

private:
  std::string mailbox_;
  std::string username_;
  std::string password_;
  std::string mailaddr_;
  MutexPtr mutex_;
};

using UserPtr = std::shared_ptr<User>;

#endif // USER_H
