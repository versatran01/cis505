#include "pop3server.h"
#include "loguru.hpp"

#include <openssl/md5.h>

#include <algorithm>
#include <experimental/filesystem>

void computeDigest(char *data, int dataLengthBytes,
                   unsigned char *digestBuffer) {
  /* The digest will be written to digestBuffer, which must be at least
   * MD5_DIGEST_LENGTH bytes long */

  MD5_CTX c;
  MD5_Init(&c);
  MD5_Update(&c, data, dataLengthBytes);
  MD5_Final(digestBuffer, &c);
}

Pop3Server::Pop3Server(int port_no, int backlog, bool verbose,
                       const std::string &mailbox)
    : Server(port_no, backlog, verbose), mailbox_(mailbox) {
  if (mailbox_.empty()) {
    LOG_F(ERROR, "No mailbox, dir={%s}", mailbox_.c_str());
  } else {
    LOG_F(INFO, "Mailbox, dir={%s}", mailbox_.c_str());
  }
}

void Pop3Server::Mailbox() {
  // Current path is
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "Current path, dir={%s}", cwd.c_str());

  // Mailbox dir is
  fs::path mailbox_dir = cwd / mailbox_;

  // Check if it is a directory
  if (fs::is_directory(mailbox_dir)) {
    LOG_F(INFO, "Mailbox, path={%s}", mailbox_dir.c_str());
    // Read all users
    for (const fs::directory_entry &file :
         fs::directory_iterator(mailbox_dir)) {
      if (file.path().extension().string() == ".mbox") {
        const auto &name = file.path().stem().string();
        users_.emplace_back(name, file.path().string());
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

void Pop3Server::Work(SocketPtr sock_ptr) {
  //
  //
}
