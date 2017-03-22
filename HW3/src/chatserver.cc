#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>

#include "cmdparser.hpp"
#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"
#include "rang.hpp"

#include "chatutils.hpp"

static constexpr int kMaxBufSize = 1024;

void ConfigureParser(cli::Parser &parser) {
  parser.set_required<std::vector<std::string>>(
      "", "config file and index",
      "Config file of forwarding address and index of server");
  parser.set_optional<bool>("v", "verbose", false,
                            "Print to stdout when sth important happnes.");
  parser.set_optional<bool>("l", "log to file", false,
                            "Log to file if set, otherwise log to stderr");
  parser.set_optional<std::string>("o", "ordering mode", "unordered",
                                   "Ordering mode: unordered, fifo, total");
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

  // Get config file and index
  const auto config_and_index = parser.get<std::vector<std::string>>("");
  if (config_and_index.size() != 2) {
    LOG_F(ERROR, "Wrong number of positional arguments.");
    return EXIT_FAILURE;
  }
  const auto config = config_and_index[0];
  const auto index = std::atoi(config_and_index[1].c_str());
  LOG_F(INFO, "argv: %s %d", config.c_str(), index);

  // Check index
  if (index <= 0) {
    LOG_F(ERROR, "Invalid server index.");
    return EXIT_FAILURE;
  }

  std::vector<ServerAddrPort> servers;
  try {
    servers = ParseConfig(config);
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, err.what());
    return EXIT_FAILURE;
  }
  LOG_F(INFO, "Total servers, n={%zu}", servers.size());

  // Setup connection
  const auto &server = servers[index - 1];
  const auto &binding = server.binding();
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    LOG_F(ERROR, "[S%d] Failed to create socket", index);
    return EXIT_FAILURE;
  }

  // Bind to binding address
  auto serv_addr = MakeSockAddrInet(binding);
  if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    LOG_F(ERROR, "[S%d] Failed to bind to address", index);
    return EXIT_FAILURE;
  }

  while (true) {
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    char buffer[kMaxBufSize];
    int nrecv = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&src, &srclen);
    if (nrecv < 0) {
      LOG_F(ERROR, "[S%d] recvfrom failed", index);
      continue;
    }

    // Null terminate buffer
    buffer[nrecv] = 0;
    std::string msg(buffer, nrecv);
    LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}", index, msg.c_str(), nrecv);

    sendto(sock, buffer, nrecv, 0, (struct sockaddr *)&src, sizeof(src));
  }

  return EXIT_SUCCESS;
}
