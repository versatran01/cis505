#include <iostream>
#include <regex>
#include <string>

using namespace std;

int main() {
  std::string mail_addr_pattern("[[:alnum:]]+[[:alnum:].]*[[:alnum:]]+");
  mail_addr_pattern = mail_addr_pattern + "@" + mail_addr_pattern;
  std::string mail_from_pattern = "MAIL FROM:[ ]*<(" + mail_addr_pattern + ")>";
  std::regex mail_from_regex(mail_from_pattern, std::regex::icase);

  std::string rcpt_to_pattern = "RCPT TO:[ ]*<(" + mail_addr_pattern + ")>";
  std::regex rcpt_to_regex(rcpt_to_pattern, std::regex::icase);

  const std::string mail_from_str("   mAil From:   <test.fuck@shit> ");
  smatch results;

  if (regex_search(mail_from_str, results, mail_from_regex))
    cout << results.str(1) << endl;
}
