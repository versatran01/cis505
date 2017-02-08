#include <argp.h>
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
  struct argp_option options[] = {{0, 'p', "PORT_NO", 0, "Port number."},
                                  {0, 'a', 0, 0, "Print name and seas login."},
                                  {0, 'v', 0, 0, "Verbose mode."},
                                  {0}};
  struct argp argp = {options, parse_opt, 0, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &args);

  if (status)
    fatal("Failed to parse arguments, error code: %d.", status);

  if (args.port_no < 1024)
    cmdLineErr("Port number %d is previlaged.", args.port_no);

  return args;
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto args = ParseCmdArguments(argc, argv);

  if (args.print_name) {
    fprintf(stdout, "Chao Qu / quchao\n");
    exit(EXIT_SUCCESS);
  }

  LOG_F(INFO, "Port number %d", args.port_no);
  return EXIT_SUCCESS;
}
