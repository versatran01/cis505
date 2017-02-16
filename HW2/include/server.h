#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <string>
#include <vector>

using SocketPtr = std::shared_ptr<int>;

/**
 * @brief Remove closed sockets
 */
void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs);

/**
 * @brief Write one line, return success or not
 */
bool WriteLine(int fd, std::string line);

/**
 * @brief Read one line, return success or not
 */
bool ReadLine(int fd, std::string &line);

typedef void (*sa_handler_ptr)(int);
/**
 * @brief SetSigintHandler
 */
void SetSigintHandler(sa_handler_ptr handler);

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
  int listen_fd_;
  std::vector<SocketPtr> open_sockets_;

private:
  int port_no_;
  int backlog_;

protected:
  bool verbose_;
};

#endif // SERVER_H
