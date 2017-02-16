#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>
#include <vector>

using SocketPtr = std::shared_ptr<int>;

/**
 * @brief RemoveClosedSockets
 * @param socket_ptrs
 */
void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs);

/**
 * @brief WriteLine
 * @param fd
 * @param line
 * @return
 */
bool WriteLine(int fd, std::string line);

/**
 * @brief ReadLine
 * @param fd
 * @param line
 * @return
 */
bool ReadLine(int fd, std::string &line);

/**
 * @brief ExtractCommand
 * @param request
 * @return
 */
std::string ExtractCommand(std::string request, size_t len = 4);

/**
 * @brief The Server class, base class
 */
class Server {
public:
  Server(int port_no, int backlog, bool verbose);
  virtual ~Server() = default;

  // Disable copy constructor and copy-assignment operator
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;

  void Setup();
  void Run();

  virtual void Work(SocketPtr &sock_ptr) = 0;
  virtual void Stop() = 0;

protected:
  bool verbose_;
  int listen_fd_;
  std::vector<SocketPtr> open_sockets_;

private:
  int port_no_;
  int backlog_;
};

#endif // SERVER_H
