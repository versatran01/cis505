#include <argp.h>

#include "lpi.h"
#include "smtpserver.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

SmtpServer *smtp_server_ptr = nullptr;

void SigintHandler(int sig) {
  smtp_server_ptr->Stop();
  exit(EXIT_SUCCESS);
}

static char args_doc[] = "MAILBOX";

struct argp_args {
  int port_no = 2500;   // port number, default is 2500
  int print_name = 0;   // print name and seas login to stderr
  bool verbose = false; // verbose mode, log to stderr, otherwise log to file
  int backlog = 10;     // backlog option to listen
  const char *mailbox;  // directory of mailbox
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
  case ARGP_KEY_ARG:
    // Too many arguments
    if (state->arg_num >= 1)
      argp_usage(state);

    args->mailbox = arg;
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
      args->mailbox = "\0";
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

argp_args ParseCmdArguments(int argc, char **argv) {
  // Parse command line arguments
  struct argp_args args;
  struct argp argp_argp = {options, parse_opt, args_doc, 0};
  int status = argp_parse(&argp_argp, argc, argv, 0, 0, &args);

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
    loguru::add_file("smtp.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  SetSigintHandler(SigintHandler);

  // EchoServer
  SmtpServer smtp_server(args.port_no, args.backlog, args.verbose,
                         args.mailbox);
  smtp_server_ptr = &smtp_server;

  smtp_server.Setup();
  smtp_server.Mailbox();
  smtp_server.Run();

  return EXIT_SUCCESS;
}
