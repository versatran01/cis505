#include "mail.h"

#include <algorithm>

void Mail::Reset() {
  sender_.clear();
  recipients_.clear();
}

bool Mail::RecipientExists(const std::string &mail_addr) const {
  return std::find(recipients_.begin(), recipients_.end(), mail_addr) !=
         recipients_.end();
}
