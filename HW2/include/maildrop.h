#ifndef MAILDROP_H
#define MAILDROP_H

#include "mail.h"

class Maildrop {
public:
  Maildrop() = default;

  const std::vector<Mail> &mails() const { return mails_; }
  size_t NumMails() const { return mails_.size(); }

  void AddMail(const Mail &mail) { mails_.push_back(mail); }
  void Clear() { mails_.clear(); }
  size_t TotalOctets() const;
  const Mail &GetMail(size_t i) const { return mails_[i]; }

private:
  std::vector<Mail> mails_;
};

#endif // MAILDROP_H
