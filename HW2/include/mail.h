#ifndef MAIL_H
#define MAIL_H

#include <chrono>
#include <string>
#include <vector>

class Mail {
public:
  using TimePointSys = std::chrono::time_point<std::chrono::system_clock>;

  Mail() = default;

  const std::string &sender() const { return sender_; }
  const std::vector<std::string> &recipients() const { return recipients_; }
  const TimePointSys &time() const { return time_; }

  void set_sender(const std::string &from) { sender_ = from; }
  void AddRecipient(const std::string &to) { recipients_.push_back(to); }
  void Reset();

  bool RecipientExists(const std::string &mail_addr) const;

private:
  std::string sender_;
  std::vector<std::string> recipients_;
  TimePointSys time_;
};

#endif // MAIL_H
