#ifndef SERVER_H
#define SERVER_H

#include <memory>
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

class Server {
public:
private:
  bool verbose_;
  int listen_fd_;
  std::vector<SocketPtr> open_connections_;
};

#endif // SERVER_H
