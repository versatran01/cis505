#ifndef MAILDROP_H
#define MAILDROP_H

#include "mail.h"

class Maildrop {
public:
  Maildrop() = default;

  const std::vector<Mail> &mails() const { return mails_; }

  void AddMail(const Mail &mail) { mails_.push_back(mail); }
  void Clear() { mails_.clear(); }

  size_t TotalOctets(bool count_deleted = false) const;
  size_t NumMails(bool count_deleted = false) const;

  const Mail &GetMail(size_t i) const { return mails_[i]; }
  Mail &GetMail(size_t i) { return mails_[i]; }

  void Update();

private:
  std::vector<Mail> mails_;
};

#endif // MAILDROP_H
