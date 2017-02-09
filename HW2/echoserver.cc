#include <argp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"
#include "lpi.h"

/**
 * @brief The arguments struct
 */
struct argp_args {
  int port_no = 10000; // port number
  int print_name = 0;  // print name and seas login to stderr
  int verbose = 0;     // verbose mode
  int backlog = 10;    // backlog option to listen
};

/**
 * @brief Parse command line options, used by argp
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct argp_args *args = (struct argp_args *)state->input;
  switch (key) {
  case 'p':
    args->port_no = std::atoi(arg);
    break;
  case 'a':
    args->print_name = 1;
    break;
  case 'v':
    args->verbose = 1;
    break;
  case 'b':
    args->backlog = std::atoi(arg);
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/**
 * @brief Parse command line arguments
 * @return argp_args struct that contains all the options and inputs
 */
argp_args ParseCmdArguments(int argc, char **argv) {
  // Parse command line arguments
  struct argp_args args;
  struct argp_option options[] = {
      {0, 'p', "PORT_NO", 0, "Port number, default is 10000."},
      {0, 'a', 0, 0, "Print name and seas login to stderr."},
      {0, 'v', 0, 0, "Verbose mode."},
      // Extra options
      {0, 'b', "BACKLOG", 0, "Number of connections on incoming queue."},
      {0}};
  struct argp argp = {options, parse_opt, 0, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &args);

  if (status)
    fatal("Failed to parse arguments, error code: %d.", status);

  if (args.port_no < 1024)
    cmdLineErr("Port number %d is previlaged.\n", args.port_no);

  return args;
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto args = ParseCmdArguments(argc, argv);

  // Print name and seas login
  if (args.print_name) {
    fprintf(stderr, "Chao Qu / quchao\n");
    exit(EXIT_SUCCESS);
  }

  // Setup log
  if (!args.verbose) {
    loguru::add_file("echoserver.log", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
  }

  LOG_F(INFO, "args: port_no={%d}", args.port_no);
  LOG_F(INFO, "args: backlog={%d}", args.backlog);

  // Create a socket
  int listen_fd;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    errExit("Server: failed to create listen socket.");
  }

  LOG_F(INFO, "Create listen socket, fd={%d}", listen_fd);

  // Prepare sockaddr
  sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(args.port_no);

  // Bind to an address
  if (bind(listen_fd, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    errExit("Server: failed to bind listen socket.");
  }

  LOG_F(INFO, "Bind listen socket, port={%d}", (int)server_addr.sin_port);

  // Listen to incoming connection
  if (listen(listen_fd, args.backlog) == -1) {
    errExit("Server: failed to listen to connections.");
  }

  LOG_F(INFO, "Start listening to connections");

  // Main accept loop
  while (true) {
    break;
  }

  return EXIT_SUCCESS;
}
