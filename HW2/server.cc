#include "server.h"

#include <algorithm>
#include <unistd.h>

void RemoveClosedSockets(std::vector<SocketPtr> &socket_ptrs) {
  auto is_socket_closed = [](const auto &fd) { return *fd < 0; };
  socket_ptrs.erase(
      std::remove_if(socket_ptrs.begin(), socket_ptrs.end(), is_socket_closed),
      socket_ptrs.end());
}

bool WriteLine(int fd, std::string line) {
  line.append("\r\n");
  const int len = line.size();
  int num_sent = 0;

  while (num_sent < len) {
    int n = write(fd, &line.data()[num_sent], len - num_sent);
    // write() failed
    if (n == -1) {
      if (errno == EINTR) {
        // Interrupted, restart write
        continue;
      }

      return false;
    }
    num_sent += n;
  }

  return true;
}

bool ReadLine(int fd, std::string &line) {
  line.clear();
  const char lf = '\n';
  const char cr = '\r';
  char ch;

  for (;;) {
    bool got_cr = false;
    // Read one char
    const auto n = read(fd, &ch, 1);

    // read() failed
    if (n == -1) {
      if (errno == EINTR) {
        // Interrupted, so there might be a signal
        continue;
      }

      return false;
    }

    // EOF, break
    if (n == 0)
      break;

    // Read succeeded
    // LF, just break
    if (ch == lf)
      break;

    // CR, dont append, mark it
    if (ch == cr) {
      got_cr = true;
      continue;
    }

    // non-EOL, so append previous CR
    if (got_cr)
      line += cr;

    // non-EOL, append
    line += ch;
  }

  return true;
}
