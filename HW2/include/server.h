#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>
#include <vector>

bool WriteLine(int fd, std::string line);
bool ReadLine(int fd, std::string &line);
std::string ExtractCommand(const std::string &request, size_t len = 4);

typedef void (*sa_handler_ptr)(int);
void SetSigintHandler(sa_handler_ptr handler);

using SocketPtr = std::shared_ptr<int>;
void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs);

class Server {
public:
  Server(int port_no, int backlog, bool verbose);
  virtual ~Server() = default;

  // Disable copy constructor and copy-assignment operator
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;

  void Setup();
  void Run();

  virtual void Work(const SocketPtr &sock_ptr) = 0;
  virtual void Stop() = 0;

protected:
  void CreateSocket();
  void ReuseAddrPort();
  void BindAddress();
  void ListenToConn();

  int listen_fd_;
  std::vector<SocketPtr> open_sockets_;

private:
  int port_no_;
  int backlog_;

protected:
  bool verbose_;
};

#endif // SERVER_H
