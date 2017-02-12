#include <argp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>

#include "lpi.h"
#include "server.h"
#include "string_algorithm.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

int listen_fd = -1;

static int v = 0;
static char args_doc[] = "MAILBOX";

/**
 * @brief The argp_args struct
 */
struct argp_args {
  int port_no = 2500; // port number, default is 2500
  int print_name = 0; // print name and seas login to stderr
  int verbose = 0;    // verbose mode, log to stderr, otherwise log to file
  int backlog = 10;   // backlog option to listen
  char *mailbox;      // directory of mailbox
};

/**
 * @brief sigint_handler
 * @param sig
 */
void SigintHandler(int sig) {
  close(listen_fd);
  LOG_F(INFO, "Close listen socket, fd={%d}, sig={SIGINT}", listen_fd);

  exit(EXIT_SUCCESS);
}

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
  case ARGP_KEY_ARG:
    // Too many arguments
    if (state->arg_num >= 1)
      argp_usage(state);
    args->mailbox = arg;
    break;
  // case ARGP_KEY_END:
  //   if (state->arg_num < 1) /* Not enough arguments. */
  //     argp_usage(state);
  //   break;
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
  struct argp argp = {options, parse_opt, args_doc, 0};
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

  // Force verbose
  args.verbose = 1;
  v = args.verbose;

  // Setup log
  if (!args.verbose) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file("echoserver.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  LOG_F(INFO, "args: port_no={%d}", args.port_no);
  LOG_F(INFO, "args: backlog={%d}", args.backlog);
  LOG_F(INFO, "args: mailbox={%s}", args.mailbox);

  // Create a socket
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    errExit("Server: failed to create listen socket.");
  }

  LOG_F(INFO, "Create listen socket, fd={%d}", listen_fd);

  // Reuse addr and port
  int enable = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) <
      0) {
    errExit("Failed to set socket option SO_REUSEADDR");
  }
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) <
      0) {
    errExit("Failed to set socket option SO_REUSEPORT");
  }

  LOG_F(INFO, "Set socket opt to reuse addr and port, fd={%d}", listen_fd);

  // Setup SIGINT handler
  struct sigaction sa;
  sa.sa_handler = SigintHandler;
  sa.sa_flags = 0; // or SA_RESTART
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    errExit("Failed to set SIGINT handler");
  }

  LOG_F(INFO, "Set SIGINT handler");

  // Prepare sockaddr
  sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(args.port_no);

  // Bind to an address
  int ret = bind(listen_fd, (sockaddr *)&server_addr, sizeof(server_addr));
  if (ret == -1) {
    errExit("Server: failed to bind listen socket.");
  }

  LOG_F(INFO, "Bind listen socket, port={%d}", (int)server_addr.sin_port);

  // Listen to incoming connection
  if (listen(listen_fd, args.backlog) == -1) {
    errExit("Server: failed to listen to connections.");
  }

  LOG_F(INFO, "Start listening to connections");

  sockaddr_in client_addr;
  socklen_t sin_size = sizeof(client_addr);
  char client_ip[INET_ADDRSTRLEN];

  std::vector<SocketPtr> open_sockets;
  open_sockets_ptr = &open_sockets;
  // Main accept loop
  while (true) {
    // Accept connection
    int connect_fd = accept(listen_fd, (sockaddr *)&client_addr, &sin_size);
    if (connect_fd == -1) {
      LOG_F(WARNING, "Accept failed, port={%d}", (int)server_addr.sin_port);
    }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip));
    LOG_F(INFO, "New connection, fd={%d}, ip={%s}, port={%d}", connect_fd,
          client_ip, (int)client_addr.sin_port);

    // DEBUG_PRINT
    if (v)
      fprintf(stderr, "[%d] New connection\n", connect_fd);

    auto connect_fd_ptr = std::make_shared<int>(connect_fd);
    open_sockets.push_back(connect_fd_ptr);

    // Create a thread to handle connection and detach
    std::thread worker(HandleConnection, connect_fd_ptr);
    worker.detach();

    // Clean closed sockets
    RemoveClosedSockets(open_sockets);
  }

  return EXIT_SUCCESS;
}
