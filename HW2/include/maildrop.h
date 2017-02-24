#ifndef MAILDROP_H
#define MAILDROP_H

#include "mail.h"

class MailDrop {
public:
  MailDrop() = default;

  const std::vector<Mail> &mails() const { return mails_; }
  size_t NumMails() const { return mails_.size(); }

private:
  std::vector<Mail> mails_;
  std::vector<int> delete_mask_;
};

#endif // MAILDROP_H
