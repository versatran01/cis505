#include "maildrop.h"

size_t Maildrop::TotalOctets() const {
  size_t n = 0;
  for (const Mail &mail : mails_) {
    n += mail.Octets();
  }
  return n;
}
