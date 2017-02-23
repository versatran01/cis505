#include "user.h"
#include <fstream>
#include <iostream>

User::User(const std::string &mailbox, const std::string &username)
    : mailbox_(mailbox), username_(username), password_("cis505"),
      mail_addr_(username + "@localhost"),
      mutex_(std::make_shared<std::mutex>()) {}

void User::WriteMail(const Mail &mail) const {
  std::lock_guard<std::mutex> guard(*mutex_);

  std::ofstream mbox_file;
  mbox_file.open(mailbox_, std::ios::out | std::ios::app);
  mbox_file << "From <" << mail.sender() << "> " << mail.TimeStr();
  for (const auto &line : mail.data()) {
    mbox_file << line << "\n";
  }
  mbox_file << "\n";
  mbox_file.close();
}
