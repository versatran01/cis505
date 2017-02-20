#include <argp.h>
#include <cstring>
#include <thread>

#include "echoserver.h"
#include "lpi.h"
#include "string_algorithms.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

EchoServer *echo_server_ptr = nullptr;

void EchoServer::Work(SocketPtr sock_ptr) {
  auto fd = *sock_ptr;
  LOG_F(INFO, "[%d] Inside EchoServer::Work", fd);

  // Send greeting
  const auto greeting = "+OK Server ready (Author: Chao Qu / quchao)";
  WriteLine(fd, greeting);

  // Handle client
  while (true) {
    std::string request;
    ReadLine(fd, request);
    trim(request);
    LOG_F(INFO, "[%d] Read, str={%s}", fd, request.c_str());

    // Extract command
    const auto command = ExtractCommand(request);
    LOG_F(INFO, "[%d] Extract command, cmd={%s}", fd, command.c_str());

    // Check if it is ECHO or QUIT
    if (command == "ECHO") {
      auto text = request.substr(5);
      trim_front(text);
      auto response = std::string("+OK ") + text;
      WriteLine(fd, response);
      LOG_F(INFO, "[%d] Write, str={%s}", fd, response.c_str());

    } else if (command == "QUIT") {
      const auto response = "+OK Goodbye!";
      WriteLine(fd, response);
      LOG_F(INFO, "[%d] Write, str={%s}", fd, response);

      // Close socket and mark it as closed
      close(fd);
      LOG_F(INFO, "[%d] Connection closed", fd);

      *sock_ptr = -1;
      return;

    } else {
      const auto response = "-ERR Unknown command";
      WriteLine(fd, response);
      LOG_F(INFO, "[%d] Write, str={%s}", fd, response);
    }
  }
}

void SigintHandler(int sig) {
  CHECK_F(echo_server_ptr != nullptr, "Forget to set pointer");
  echo_server_ptr->Stop();
  exit(EXIT_SUCCESS);
}

struct argp_args {
  int port_no = 10000;  // port number, default is 10000
  int print_name = 0;   // print name and seas login to stderr
  bool verbose = false; // verbose mode, log to stderr, otherwise log to file
  int backlog = 10;     // backlog option to listen
};

struct argp_option options[] = {
    {0, 'p', "PORT_NO", 0, "Port number, default is 10000."},
    {0, 'a', 0, 0, "Print name and seas login to stderr."},
    {0, 'v', 0, 0, "Verbose mode."},
    // Extra options
    {0, 'b', "BACKLOG", 0, "Number of connections on incoming queue."},
    {0}};

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
    args->verbose = true;
    break;
  case 'b':
    args->backlog = std::atoi(arg);
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

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
