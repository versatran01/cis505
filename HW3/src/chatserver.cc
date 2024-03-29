#include "cmdparser.hpp"
#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"
#include "rang.hpp"

#include "servernode.hpp"

void ConfigureParser(cli::Parser &parser) {
  parser.set_required<std::vector<std::string>>(
      "", "config file and id",
      "Config file of forwarding address and id of server");
  parser.set_optional<bool>("v", "verbose", false,
                            "Print to stdout when sth important happnes.");
  parser.set_optional<bool>("v", "log to file", false,
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

  // Get config file and index
  const auto config_and_id = parser.get<std::vector<std::string>>("");
  if (config_and_id.size() != 2) {
    LOG_F(ERROR, "Wrong number of positional arguments.");
    return EXIT_FAILURE;
  }
  const auto config = config_and_id[0];
  const auto id = std::atoi(config_and_id[1].c_str());
  //  LOG_F(INFO, "argv: %s %d", config.c_str(), id);

  // Get order mode
  const auto order = parser.get<std::string>("o");

  // Check index
  if (id <= 0) {
    LOG_F(ERROR, "Invalid server index.");
    return EXIT_FAILURE;
  }

  // Setup logging
  if (!parser.get<bool>("v")) {
    const auto log_name = "chatserver" + std::to_string(id) + ".log";
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file(log_name.c_str(), loguru::Truncate, loguru::Verbosity_MAX);
  }

  // Construct and run server
  ServerNode node(id, order);
  node.Init(config);
  node.Run();

  return EXIT_SUCCESS;
}
