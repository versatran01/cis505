#ifndef USER_H
#define USER_H

#include <memory>
#include <mutex>
#include <string>

#include "mail.h"

class User {
public:
  using MutexPtr = std::shared_ptr<std::mutex>;

  User(const std::string &name, const std::string &mbox);

  const std::string &name() const { return name_; }
  const std::string &mbox() const { return mbox_; }
  const std::string &addr() const { return addr_; }

  void WriteMail(const Mail &mail) const;

private:
  std::string name_;
  std::string mbox_;
  std::string addr_;
  MutexPtr mutex_;
};

#endif // USER_H
