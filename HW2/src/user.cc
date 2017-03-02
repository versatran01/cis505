#include "user.h"
#include "loguru.hpp"

#include <fstream>
#include <iostream>
#include <regex>

User::User(const std::string &mailbox, const std::string &username)
    : mailbox_(mailbox), username_(username), password_("cis505"),
      mailaddr_(username + "@localhost"),
      mutex_(std::make_shared<std::mutex>()) {}

void User::WriteMail(const Mail &mail) const {
  std::ofstream mbox_file;
  mbox_file.open(mailbox_, std::ios::out | std::ios::app);
  mbox_file << "From <" << mail.sender() << "> " << mail.TimeStr();
  for (const auto &line : mail.lines()) {
    mbox_file << line << "\n";
  }
  mbox_file.close();
}

Maildrop User::ReadMaildrop() const {
  Maildrop maildrop;

  std::string mailaddr_pattern("[[:alnum:]]+[[:alnum:].]*");
  mailaddr_pattern = mailaddr_pattern + "@" + mailaddr_pattern;
  std::string datetime_pattern("[[:alnum:][:space:]\\:]+");

  std::string mail_head_pattern =
      "From <(" + mailaddr_pattern + ")> (" + datetime_pattern + ")";
  std::regex mail_head_regex(mail_head_pattern);

  std::string line;
  std::ifstream mbox_file(mailbox_);
  Mail mail;
  while (std::getline(mbox_file, line)) {
    if (line.substr(0, 5) == "From ") {
      // Got the first line
      std::smatch results;
      if (std::regex_search(line, results, mail_head_regex)) {
        // This is a new mail
        // Add old mail first
        if (!mail.Empty())
          maildrop.AddMail(mail);
        // Prepare for new mail
        mail.Clear();
        mail.set_sender(results.str(1));
        mail.SetTimeFromString(results.str(2));
        mail.AddRecipient(mailaddr()); // This user is the recipient
        continue;
      }
    }

    // Everthing else is a line of the mail
    mail.AddLine(line);
  }

  maildrop.AddMail(mail);
  return maildrop;
}

void User::ClearMailbox() const {
  std::ofstream mbox_file;
  mbox_file.open(mailbox_, std::ios::out | std::ios::trunc);
  mbox_file.close();
}
