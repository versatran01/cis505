#ifndef MAIL_H
#define MAIL_H

#include <chrono>
#include <string>
#include <vector>

using TimePointSys = std::chrono::time_point<std::chrono::system_clock>;

/**
 * @brief The Mail class, a single mail
 */
class Mail {
public:
  Mail() = default;
  explicit Mail(const std::string &sender);

  /// Getters
  const std::string &sender() const { return sender_; }
  const std::vector<std::string> &recipients() const { return recipients_; }
  const TimePointSys &time() const { return time_; }
  const std::vector<std::string> &lines() const { return lines_; }
  bool deleted() const { return deleted_; }

  /// Setters
  void set_sender(const std::string &sender) { sender_ = sender; }
  void set_time(const TimePointSys &time) { time_ = time; }

  void SetTimeFromString(const std::string &time_str);
  void AddRecipient(const std::string &recipient) {
    recipients_.push_back(recipient);
  }
  void AddLine(const std::string &line) { lines_.push_back(line); }
  void MarkDeleted() const { deleted_ = true; }
  void MarkUndeleted() const { deleted_ = false; }

  /**
   * @brief Check if recipient already exists
   * @param mailaddr, recipient's mail address
   * @return True if exists
   */
  bool RecipientExists(const std::string &mailaddr) const;
  void Clear();

  /**
   * @brief Check if mail is empty
   * @return True if empty
   */
  bool Empty() const;

  /**
   * @brief Timestamp this mail with the current time
   */
  void Stamp() { time_ = std::chrono::system_clock::now(); }

  /**
   * @brief TimerStr
   * @return time as string
   */
  std::string TimeStr() const;
  size_t Octets() const;
  std::string Data() const;

private:
  std::string sender_;                  // sender's mail address
  std::vector<std::string> recipients_; // recipients' mail address
  std::vector<std::string> lines_;      // each line of the mail's content
  TimePointSys time_;                   // time of recieveing
  mutable bool deleted_ = false;        // Whether to delete this mail
};

#endif // MAIL_H
