#ifndef USER_H
#define USER_H

#include <mutex>
#include <string>

class User {
public:
  User(const std::string &name, const std::string &mbox);

  const std::string &name() const { return name_; }
  const std::string &mbox() const { return mbox_; }

private:
  std::string name_;
  std::string mbox_;
};

#endif // USER_H
