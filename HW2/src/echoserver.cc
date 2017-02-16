#include <argp.h>
#include <cstring>
#include <thread>

#include "echoserver.h"
#include "lpi.h"
#include "string_algorithm.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

EchoServer *echo_server_ptr = nullptr;

std::string EchoServer::ExtractCommand(const std::string &request, size_t len) {
  // Extract one more char
  auto command = request.substr(0, len + 1);

  // Convert to upper case
  to_upper(command);

  // Trim back
  trim_back(command);
  return command;
}

void EchoServer::Work(SocketPtr &sock_ptr) {
  LOG_F(INFO, "Inside EchoServer::Work");
  auto fd = *sock_ptr;

  // Send greeting
  auto greeting = "+OK Server ready (Author: Chao Qu / quchao)";
  WriteLine(fd, greeting);

  // Handle client
  while (true) {
    std::string request;
    ReadLine(fd, request);
    trim(request);

    // DEBUG_PRINT
    if (verbose_)
      fprintf(stderr, "[%d] C: %s\n", fd, request.c_str());
    LOG_F(INFO, "Read from fd={%d}, str={%s}", fd, request.c_str());

    // Extract command
    auto command = ExtractCommand(request);

    // Check if it is ECHO or QUIT
    if (command == "ECHO") {
      auto text = request.substr(5);
      trim_front(text);
      auto response = std::string("+OK ") + text;
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={ECHO}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
    } else if (command == "QUIT") {
      auto response = std::string("+OK Goodbye!");
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={QUIT}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
      if (verbose_)
        fprintf(stderr, "[%d] Connection closed\n", fd);

      // Close socket and mark it as closed
      close(fd);
      *sock_ptr = -1;
      return;
    } else {
      auto response = std::string("-ERR Unknown command");
      WriteLine(fd, response);

      // DEBUG_PRINT
      if (verbose_)
        fprintf(stderr, "[%d] S: %s\n", fd, response.c_str());
      LOG_F(INFO, "cmd={UNKNOWN}, Write to fd={%d}, str={%s}", fd,
            response.c_str());
    }
  }
}

void EchoServer::Stop() {
  // Close listen socket
  close(listen_fd_);
  LOG_F(INFO, "Close listen socket, fd={%d}, sig={SIGINT}", listen_fd_);

  RemoveClosedSockets(open_sockets_);
  LOG_F(INFO, "Remove closed sockets, num_fd_open={%d}",
        static_cast<int>(open_sockets_.size()));

  const std::string response("-ERR Server shutting down");
  for (const auto fd_ptr : open_sockets_) {
    WriteLine(*fd_ptr, response);
    close(*fd_ptr);
    LOG_F(INFO, "Close client socket, fd={%d}", *fd_ptr);
  }
}

/**
 * @brief SigintHandler
 */
void SigintHandler(int sig) {
  echo_server_ptr->Stop();
  exit(EXIT_SUCCESS);
}

/**
 * @brief The argp_args struct
 */
struct argp_args {
  int port_no = 10000; // port number, default is 10000
  int print_name = 0;  // print name and seas login to stderr
  int verbose = 0;     // verbose mode, log to stderr, otherwise log to file
  int backlog = 10;    // backlog option to listen
};

struct argp_option options[] = {
    {0, 'p', "PORT_NO", 0, "Port number, default is 10000."},
    {0, 'a', 0, 0, "Print name and seas login to stderr."},
    {0, 'v', 0, 0, "Verbose mode."},
    // Extra options
    {0, 'b', "BACKLOG", 0, "Number of connections on incoming queue."},
    {0}};

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
  struct argp argp_ = {options, parse_opt, 0, 0};
  int status = argp_parse(&argp_, argc, argv, 0, 0, &args);

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
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file("echoserver.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  SetSigintHandler(SigintHandler);

  // EchoServer
  EchoServer echo_server(args.port_no, args.backlog, args.verbose);
  echo_server_ptr = &echo_server;

  echo_server.Setup();
  echo_server.Run();

  return EXIT_SUCCESS;
}
