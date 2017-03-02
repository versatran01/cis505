#include "maildrop.h"

size_t Maildrop::TotalOctets(bool count_deleted) const {
  size_t n = 0;
  for (const Mail &mail : mails_) {
    if (count_deleted || !mail.deleted()) {
      n += mail.Octets();
    }
  }
  return n;
}

size_t Maildrop::NumMails(bool count_deleted) const {
  size_t n = 0;
  for (const Mail &mail : mails_) {
    if (count_deleted || !mail.deleted()) {
      ++n;
    }
  }
  return n;
}
