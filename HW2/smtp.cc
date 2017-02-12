#include <argp.h>

#include <cstdio>
#include <cstdlib>

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"
#include "lpi.h"

int listen_fd = -1;

static char args_doc[] = "MAILBOX";

/**
 * @brief The argp_args struct
 */
struct argp_args {
  int port_no = 2500;  // port number, default is 2500
  int print_name = 0;  // print name and seas login to stderr
  int verbose = 0;     // verbose mode, log to stderr, otherwise log to file
  int backlog = 10;    // backlog option to listen
  char *mailbox;       // directory of mailbox
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
    case ARGP_KEY_ARG:
      // Too many arguments
      if (state->arg_num >= 1) argp_usage(state);
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

  if (status) fatal("Failed to parse arguments, error code: %d.", status);

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

  // Setup log
  if (!args.verbose) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::add_file("echoserver.log", loguru::Truncate, loguru::Verbosity_MAX);
  }

  LOG_F(INFO, "args: port_no={%d}", args.port_no);
  LOG_F(INFO, "args: backlog={%d}", args.backlog);
  LOG_F(INFO, "args: mailbox={%s}", args.mailbox);

  return EXIT_SUCCESS;
}
