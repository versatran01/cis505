#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

bool WriteLine(int fd, const std::string &line);
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

  virtual void Work(SocketPtr sock_ptr) = 0;
  virtual void Stop();

protected:
  void CreateSocket();
  void ReuseAddrPort();
  void BindAddress();
  void ListenSocket();

  int listen_fd_;
  std::mutex open_sockects_mutex_;
  std::vector<SocketPtr> open_sockets_;

private:
  int port_no_;
  int backlog_;

protected:
  bool verbose_;
};

#endif // SERVER_H
