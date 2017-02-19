#include "server.h"
#include "loguru.hpp"
#include "lpi.h"
#include "string_algorithms.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <thread>

std::string ExtractCommand(const std::string &request, size_t len) {
  // Extract one more character
  auto command = request.substr(0, len + 1);

  to_upper(command);

  trim_back(command);
  return command;
}

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

void SetSigintHandler(sa_handler_ptr handler) {
  // Setup SIGINT handler
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0; // or SA_RESTART
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    const auto msg = "Failed to set SIGINT handler";
    LOG_F(ERROR, msg);
    errExit(msg);
  }
  LOG_F(INFO, "Set SIGINT handler");
}

Server::Server(int port_no, int backlog, bool verbose)
    : port_no_(port_no), backlog_(backlog), verbose_(verbose) {
  LOG_F(INFO, "port_no={%d}", port_no_);
  LOG_F(INFO, "backlog={%d}", backlog_);
}

void Server::CreateSocket() {
  // Create listen socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1) {
    const auto msg = "Failed to create listen socket";
    LOG_F(ERROR, msg);
    errExit(msg);
  }

  LOG_F(INFO, "Create listen socket, fd={%d}", listen_fd_);
}

void Server::ReuseAddrPort() {
  // Reuse address
  const int val = 1;
  constexpr auto int_size = sizeof(int);
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &val, int_size) < 0) {
    const auto msg = "Failed to set socket option SO_REUSEADDR";
    LOG_F(WARNING, msg);
  } else {
    LOG_F(INFO, "Set socket opt to reuse address, fd={%d}", listen_fd_);
  }

  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &val, int_size) < 0) {
    const auto msg = "Failed to set socket option SO_REUSEPORT";
    LOG_F(WARNING, msg);
  } else {
    LOG_F(INFO, "Set socket opt to reuse port, fd={%d}", listen_fd_);
  }
}

void Server::BindAddress() {
  // Prepare sockaddr
  sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(port_no_);

  // Bind to an address
  int ret = bind(listen_fd_, (sockaddr *)&server_addr, sizeof(server_addr));
  if (ret == -1) {
    const auto msg = "Failed to bind listen socket";
    LOG_F(ERROR, msg);
    errExit(msg);
  }

  LOG_F(INFO, "Bind listen socket, port_h={%d}, port_n={%d}", port_no_,
        static_cast<int>(server_addr.sin_port));
}

void Server::ListenToConn() {
  // Listen to incoming connection
  if (listen(listen_fd_, backlog_) == -1) {
    const auto msg = "Failed to listen to connections";
    LOG_F(FATAL, msg);
    errExit(msg);
  }

  LOG_F(INFO, "Start listening to connections");
}

void Server::Setup() {
  CreateSocket();
  ReuseAddrPort();
  BindAddress();
  ListenToConn();
}

void Server::Run() {
  sockaddr_in client_addr;
  socklen_t sin_size = sizeof(client_addr);
  char client_ip[INET_ADDRSTRLEN];

  while (true) {
    // Accept connection
    int connect_fd = accept(listen_fd_, (sockaddr *)&client_addr, &sin_size);
    if (connect_fd == -1) {
      LOG_F(WARNING, "Accept failed, fd={%d}, port_h={%d}", listen_fd_,
            port_no_);
      continue;
    }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip));
    LOG_F(INFO, "New connection, fd={%d}, ip={%s}, port_n={%d}", connect_fd,
          client_ip, static_cast<int>(client_addr.sin_port));

    // DEBUG_PRINT
    if (verbose_)
      fprintf(stderr, "[%d] New connection\n", connect_fd);

    const auto connect_fd_ptr = std::make_shared<int>(connect_fd);
    open_sockets_.push_back(connect_fd_ptr);

    // Create a thread to handle connection and detach
    std::thread worker([&] { Work(connect_fd_ptr); });
    worker.detach();

    // Clean closed sockets
    RemoveClosedSockets(open_sockets_);
    LOG_F(INFO, "Remaining open connections, num={%zu}", open_sockets_.size());
  }
}
