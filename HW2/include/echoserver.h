#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "server.h"

class EchoServer : public Server {
public:
  using Server::Server;

  virtual void Work(SocketPtr sock_ptr) override;
};

#endif // ECHOSERVER_H
