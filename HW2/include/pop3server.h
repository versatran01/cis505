#ifndef POP3SERVER_H
#define POP3SERVER_H

#include "mailserver.h"

class Pop3Server : public MailServer {
public:
  Pop3Server(int port_no, int backlog, bool verbose,
             const std::string &mailbox);

  virtual void Work(SocketPtr sock_ptr) override;
};

#endif // POP3SERVER_H
