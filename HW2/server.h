#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <vector>

using SocketPtr = std::shared_ptr<int>;
void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs);

#endif // SERVER_H
