#include "maildrop.h"

size_t Maildrop::TotalOctets() const {
  size_t n = 0;
  for (const Mail &mail : mails_) {
    if (!mail.deleted()) {
      n += mail.Octets();
    }
  }
  return n;
}

size_t Maildrop::NumMails() const {
  size_t n = 0;
  for (const Mail &mail : mails_) {
    if (!mail.deleted()) {
      ++n;
    }
  }
  return n;
}
