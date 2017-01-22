#include <argp.h>

#include <cstdlib>
#include <vector>

#ifdef DEBUG
#define DEBUG_PRINT(...)          \
  do {                            \
    fprintf(stderr, __VA_ARGS__); \
  } while (false);
#else
#define DEBUG_PRINT(...) \
  do {                   \
  } while (false)
#endif

static char args_doc[] = "FILE [FILES...]";

struct arguments {
  int num_processes = 4;
  char *file;  // need at least 1 file
  char **files;
};

/// Parse a single option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = (struct arguments *)state->input;
  switch (key) {
    case 'n':
      arguments->num_processes = std::atoi(arg);
      break;
    case ARGP_KEY_NO_ARGS:
      argp_usage(state);
    case ARGP_KEY_ARG:
      arguments->file = arg;
      arguments->files = &state->argv[state->next];
      state->next = state->argc;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  struct arguments arguments;
  struct argp_option options[] = {
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."}, {0}};
  struct argp argp = {options, parse_opt, args_doc, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &arguments);

  if (status) {
    fprintf(stderr, "Failed to parse arguments, error code: %d", status);
    exit(status);
  }

  DEBUG_PRINT("Number of processes: %d\n", arguments.num_processes);
  DEBUG_PRINT("Input files: %s", arguments.file);
  for (int i = 0; arguments.files[i]; ++i) {
    DEBUG_PRINT(", %s", arguments.files[i]);
  }
  DEBUG_PRINT("\n")


  // Read data from files

  return 0;
}
