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

#endif // SERVER_H
