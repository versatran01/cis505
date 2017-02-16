#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "server.h"

/**
 * @brief The EchoServer class
 */
class EchoServer : public Server {
public:
  using Server::Server;

  virtual void Work(const SocketPtr &sock_ptr) override;
  virtual void Stop() override;

private:
  std::string ExtractCommand(const std::string &request, size_t len = 4);
};

#endif // ECHOSERVER_H
