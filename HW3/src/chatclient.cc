#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <mutex>
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

  // Setup connection
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in dest;
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(4711);
  inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));
  sendto(sock, "abc", strlen("abc"), 0, (struct sockaddr *)&dest, sizeof(dest));

  char buf[100];
  struct sockaddr_in src;
  socklen_t srcSize = sizeof(src);
  int rlen =
      recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&src, &srcSize);
  buf[rlen] = 0;
  printf("Echo: [%s] (%d byptes) from %s\n", buf, rlen,
         inet_ntoa(src.sin_addr));

  close(sock);

  return 0;
}
