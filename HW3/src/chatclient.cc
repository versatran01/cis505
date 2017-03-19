#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <mutex>
#include <regex>
#include <string>

#include "cmdparser.hpp"
#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

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
  std::string pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_port_regex(pattern);
  std::smatch results;
  if (!std::regex_search(addr_port, results, addr_port_regex))
    LOG_F(FATAL, "Invalid address and port.");

  const auto addr = results.str(1);
  const auto port = std::atoi(results.str(2).c_str());
  LOG_F(INFO, "addr: %s, port: %d", addr.c_str(), port);

  // Setup connection
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, addr.c_str(), &(server_addr.sin_addr));
  sendto(sock_fd, "abc", strlen("abc"), 0, (struct sockaddr *)&server_addr,
         sizeof(server_addr));

  char buf[100];
  struct sockaddr_in src;
  socklen_t src_size = sizeof(src);
  int rlen = recvfrom(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&src,
                      &src_size);
  buf[rlen] = 0;
  printf("Echo: [%s] (%d byptes) from %s\n", buf, rlen,
         inet_ntoa(src.sin_addr));

  close(sock_fd);

  return 0;
}
