#ifndef MAIL_H
#define MAIL_H

#include <chrono>
#include <string>
#include <vector>

class Mail {
public:
  using TimePointSys = std::chrono::time_point<std::chrono::system_clock>;

  Mail() = default;

  // Getters
  const std::string &sender() const { return sender_; }
  const std::vector<std::string> &recipients() const { return recipients_; }
  const TimePointSys &time() const { return time_; }
  const std::vector<std::string> &data() const { return data_; }

  // Setters
  void set_sender(const std::string &sender) { sender_ = sender; }
  void AddRecipient(const std::string &recipient) {
    recipients_.push_back(recipient);
  }
  void AddLine(const std::string &line) { data_.push_back(line); }
  void Clear();

  // Methods
  bool RecipientExists(const std::string &mail_addr) const;
  bool Empty() const;
  void Stamp() { time_ = std::chrono::system_clock::now(); }
  std::string TimeStr() const;

private:
  std::string sender_;                  // sender's mail address
  std::vector<std::string> recipients_; // recipients' mail address
  std::vector<std::string> data_;
  TimePointSys time_;
};

#endif // MAIL_H
