#include "server.h"

#include <algorithm>

/**
 * @brief Remove closed sockets
 * @param socket_ptrs
 */
void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs) {
  auto is_socket_closed = [](const auto &fd) { return *fd < 0; };
  socket_ptrs.erase(
      std::remove_if(socket_ptrs.begin(), socket_ptrs.end(), is_socket_closed),
      socket_ptrs.end());
}
