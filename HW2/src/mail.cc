#include "mail.h"

#include <algorithm>

void Mail::Clear() {
  sender_.clear();
  recipients_.clear();
  data_.clear();
}

bool Mail::RecipientExists(const std::string &mail_addr) const {
  return std::find(recipients_.begin(), recipients_.end(), mail_addr) !=
         recipients_.end();
}

bool Mail::Empty() const {
  return sender_.empty() && recipients_.empty() && data_.empty();
}
