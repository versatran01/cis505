#include <argp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "string_algorithm.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"
#include "lpi.h"

/**
 * @brief Write a string to socket, append <CRLF>
 */
void WriteLine(int fd, std::string line) {
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

      // Failed, exit
      errExit("Read failed.");
    }
    num_sent += n;
  }
}

/**
 * @brief Read an entire line from socket
 */
std::string ReadLine(int fd) {
  std::string line;
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
        // Interrupted, restart read()
        continue;
      }

      // Failed, exit
      errExit("Read failed.");
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

  return line;
}

/**
 * @brief The arguments struct
 */
struct argp_args {
  int port_no = 10000; // port number, default is 10000
  int print_name = 0;  // print name and seas login to stderr
  int verbose = 0;     // verbose mode, log to stderr, otherwise log to file
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

  // TODO: force verbose
  args.verbose = 1;

  // Setup log
  if (!args.verbose) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file("echoserver.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  LOG_F(INFO, "args: port_no={%d}", args.port_no);
  LOG_F(INFO, "args: backlog={%d}", args.backlog);

  // Create a socket
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
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

  // Main accept loop
  while (true) {
    int connect_fd = accept(listen_fd, (sockaddr *)&client_addr, &sin_size);
    if (connect_fd == -1) {
      LOG_F(WARNING, "Accept failed, port={%d}", (int)server_addr.sin_port);
    }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip));
    LOG_F(INFO, "New connection, fd={%d}, ip={%s}, port={%d}", connect_fd,
          client_ip, (int)client_addr.sin_port);
    DEBUG_PRINT("[%d] New connection.\n", connect_fd);
    auto greeting = "+OK Server ready (Author: Chao Qu / quchao)";
    WriteLine(connect_fd, greeting);

    while (true) {
      auto request = ReadLine(connect_fd);
      trim(request);
      DEBUG_PRINT("[%d] C: %s\n", connect_fd, request.c_str());

      // Extract the first 4 chars
      auto command = request.substr(0, 4);
      // Convert to upper case
      to_upper(command);
      // Check if it is ECHO or QUIT
      if (command == "ECHO") {
        auto text = request.substr(4);
        trim_front(text);
        auto response = std::string("+OK ") + text;
        WriteLine(connect_fd, response);
        DEBUG_PRINT("[%d] S: %s\n", connect_fd, response.c_str());
      } else if (command == "QUIT") {
        auto response = std::string("+OK Goodbye!");
        WriteLine(connect_fd, response);
        DEBUG_PRINT("[%d] S: %s\n", connect_fd, response.c_str());
        close(connect_fd);
        DEBUG_PRINT("[%d] Connection closed\n", connect_fd);
        break;
      } else {
        auto response = std::string("-ERR Unknown command");
        WriteLine(connect_fd, response);
        DEBUG_PRINT("[%d] S: %s\n", connect_fd, response.c_str());
      }
    }
  }

  return EXIT_SUCCESS;
}
