#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/epoll.h>

#include "cmdparser.hpp"
#include "rang.hpp"
#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

#include "chatutils.hpp"

static constexpr int kMaxEvents = 1;
static constexpr int kBufferSize = 1024;

void ConfigureParser(cli::Parser &parser) {
  parser.set_required<std::string>("", "server address and port",
                                   "IP address and port number of server");
  parser.set_optional<bool>("l", "log to file", false,
                            "Log to file if set, otherwise log to stderr");
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "*** Author: Chao Qu (quchao)\n");
    exit(1);
  }

  // Parse command line argument
  cli::Parser parser(argc, argv);
  ConfigureParser(parser);
  parser.run_and_exit_if_error();

  // Setup logging
  if (parser.get<bool>("l")) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file("chatclient.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  // Parse server address and port
  const auto addr_port = parser.get<std::string>("");
  LOG_F(INFO, "argv: %s", addr_port.c_str());
  Address server;
  try {
    server = ParseAddress(addr_port);
    LOG_F(INFO, "addr: %s, port: %d", server.addr().c_str(), server.port());
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, err.what());
    return EXIT_FAILURE;
  }

  // Setup connection
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd == -1) {
    LOG_F(ERROR, "Failed to create socket");
    return EXIT_FAILURE;
  }
  auto server_addr = MakeSockAddrInet(server);

  // Setup epoll
  int epoll_fd = epoll_create(2);
  if (epoll_fd < 0) {
    LOG_F(ERROR, "Failed to create epoll");
    return EXIT_FAILURE;
  }
  struct epoll_event event;
  // add stdin
  event.events = EPOLLIN | EPOLLPRI | EPOLLERR;
  event.data.fd = STDIN_FILENO;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) != 0) {
    LOG_F(ERROR, "Failed to add STDIN to epoll");
    return EXIT_FAILURE;
  }

  // add socket
  event.data.fd = sock_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) != 0) {
    LOG_F(ERROR, "Failed to add socket to epoll");
    return EXIT_FAILURE;
  }

  // Setup for reading
  char buffer[kBufferSize];
  char *line = nullptr;
  size_t line_len = 0;
  struct sockaddr_in src_addr;
  socklen_t src_size = sizeof(src_addr);

  while (true) {
    // Wait for fd to be ready
    int fds = epoll_wait(epoll_fd, &event, kMaxEvents, -1);

    if (fds < 0) {
      LOG_F(FATAL, "[C%d] epoll_wait failed.", sock_fd);
    }

    if (fds == 0) {
      LOG_F(WARNING, "[C%d] epoll times out", sock_fd);
      continue;
    }

    // Handle stdin or socket
    if (event.data.fd == STDIN_FILENO) {
      // stdin is ready to read
      // Read line from stdin
      int nread = getline(&line, &line_len, stdin);
      if (nread < 0) {
        LOG_F(ERROR, "[C%d] getline failed", sock_fd);
        continue;
      }
      // Get rid of the newline, and decrement nread accordingly
      line[--nread] = 0;
      LOG_F(INFO, "[C%d] Read from stdin, str={%s}, n={%d}", sock_fd, line,
            nread);

      // Send to server
      int nsent = sendto(sock_fd, line, strlen(line), 0,
                         (struct sockaddr *)&server_addr, sizeof(server_addr));
      if (nsent < 0) {
        LOG_F(ERROR, "[C%d] sendto failed", sock_fd);
        continue;
      }
      LOG_F(INFO, "[C%d] Send line={%s}, n={%d}", sock_fd, line, nsent);

      // Make sure read and send agrees
      if (nsent != nread) {
        LOG_F(WARNING, "[C%d] nread={%d} doesn't match nsent={%d}", sock_fd,
              nread, nsent);
      }
    } else if (event.data.fd == sock_fd) {
      // socket is ready to read
      int nrecv = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&src_addr, &src_size);
      if (nrecv < 0) {
        LOG_F(ERROR, "[C%d] recvfrom failed", sock_fd);
        continue;
      }

      // Null terminate
      buffer[nrecv] = 0;
      LOG_F(INFO, "[C%d] Recv from socket, str={%s}, n={%d}", sock_fd, buffer,
            nrecv);
      const auto recv_server = MakeAddress(src_addr);
      LOG_F(INFO, "[C%d] Recv server, addr={%s}", sock_fd,
            recv_server.full_str().c_str());

      // Check whether we received from the same server
      if (recv_server != server) {
        LOG_F(WARNING, "[C%d] send_addr={%s}, recv_addr={%s}", sock_fd,
              server.full_str(), recv_server.full_str().c_str());
      }

      // Extract first token
      std::string msg(buffer, nrecv);
      const auto token = msg.substr(0, msg.find(' '));

      // Print to terminal, with RANG!!!
      if (token == "+OK") {
        std::cout << rang::fg::green << msg << rang::fg::reset << std::endl;
      } else if (token == "-ERR") {
        std::cout << rang::fg::red << msg << rang::fg::reset << std::endl;
      } else {
        std::cout << rang::fg::yellow << msg << rang::fg::reset << std::endl;
      }
    } else {
      // This shouldn't happen
      LOG_F(FATAL, "[C%d] This should not happen", sock_fd);
    }
  }

  return EXIT_SUCCESS;
}
