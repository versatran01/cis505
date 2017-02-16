#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "server.h"

/**
 * @brief The EchoServer class
 */
class EchoServer : public Server {
public:
  using Server::Server;

  virtual void Work(SocketPtr &sock_ptr) override;
  virtual void Stop() override;
};

#endif // ECHOSERVER_H
