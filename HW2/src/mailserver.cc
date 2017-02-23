#include "mailserver.h"
#include "loguru.hpp"

#include <algorithm>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

MailServer::MailServer(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : Server(port_no, backlog, verbose), mailbox_(mailbox) {
  if (mailbox_.empty()) {
    LOG_F(ERROR, "No mailbox");
  } else {
    LOG_F(INFO, "Mailbox, dir={%s}", mailbox_.c_str());
  }
}

void MailServer::LoadMailbox() {
  // Current working dir
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "Current path, dir={%s}", cwd.c_str());

  // Mailbox dir
  fs::path mailbox_dir = cwd / mailbox_;

  // Check if it is a directory
  if (fs::is_directory(mailbox_dir)) {
    LOG_F(INFO, "Mailbox, path={%s}", mailbox_dir.c_str());
    // Read all users
    for (const fs::directory_entry &file :
         fs::directory_iterator(mailbox_dir)) {
      const fs::path &mailbox = file.path();
      if (mailbox.extension().string() == ".mbox") {
        const auto &name = mailbox.stem().string();
        users_.emplace_back(mailbox.string(), name);
        LOG_F(INFO, "Add user, name={%s}", name.c_str());
      }
    }
  } else {
    LOG_F(ERROR, "Mailbox invalid, path={%s}", mailbox_dir.c_str());
  }

  if (users_.empty()) {
    LOG_F(ERROR, "No user found, mbox={%s}", mailbox_.c_str());
  }
  LOG_F(INFO, "Total user, mbox={%s}, n={%zu}", mailbox_.c_str(),
        users_.size());
}

bool MailServer::UserExistsByMailAddr(const std::string &mail_addr) const {
  auto pred = [&mail_addr](const User &user) {
    return user.mail_addr() == mail_addr;
  };
  return std::find_if(users_.begin(), users_.end(), pred) != users_.end();
}

bool MailServer::UserExistsByUsername(const std::string &username) const {
  auto pred = [&username](const User &user) {
    return user.username() == username;
  };
  return std::find_if(users_.begin(), users_.end(), pred) != users_.end();
}
