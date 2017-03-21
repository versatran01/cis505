#include <cstdio>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#include "cmdparser.hpp"
#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

namespace fs = std::experimental::filesystem;

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

  // Check config file
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "cwd, path={%s}", cwd.c_str());
  fs::path config_file = cwd / config;

  // Read server addresses
  std::string line;
  std::fstream config_file_stream(config_file);
  for (int i = 1; i < index; ++i) {
    config_file_stream.ignore(std::numeric_limits<std::streamsize>::max(),
                              '\n');
  }
  config_file_stream >> line;
  if (line.empty()) {
    LOG_F(ERROR, "[S%d] failed to load server addresses", index);
    return EXIT_FAILURE;
  }
  LOG_F(INFO, "[S%d] config={%s}", index, line.c_str());

  // Parse server addresses
  // Figure out whether we have a single address or two
  std::string forward_addr_port, bind_addr_port;
  const auto delim_index = line.find(',');
  if (delim_index == std::string::npos) {
    // Single address
    forward_addr_port = line;
    bind_addr_port = line;
  } else {
    forward_addr_port = line.substr(0, delim_index);
    bind_addr_port = line.substr(delim_index + 1);
  }
  LOG_F(INFO, "[S%d] forward={%s}, bind={%s}", index, forward_addr_port.c_str(),
        bind_addr_port.c_str());

  // Extract forward address and port
  std::string pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_port_regex(pattern);
  std::smatch results;
  if (!std::regex_search(forward_addr_port, results, addr_port_regex)) {
    LOG_F(ERROR, "Invalid forward address and port.");
    return EXIT_FAILURE;
  }

  const auto forward_addr = results.str(1);
  const auto forward_port = std::atoi(results.str(2).c_str());
  LOG_F(INFO, "[S%d] forward addr={%s}, port={%d}", index, forward_addr.c_str(),
        forward_port);

  // Extract bind address and port
  if (!std::regex_search(bind_addr_port, results, addr_port_regex)) {
    LOG_F(ERROR, "Invalid bind address and port.");
    return EXIT_FAILURE;
  }
  const auto bind_addr = results.str(1);
  const auto bind_port = std::atoi(results.str(2).c_str());
  LOG_F(INFO, "[S%d] bind addr={%s}, port={%d}", index, bind_addr.c_str(),
        bind_port);

  return EXIT_SUCCESS;
}
