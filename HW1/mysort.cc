#include <argp.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <istream>
#include <iterator>
#include <string>
#include <vector>

#ifdef DEBUG
#define DEBUG_PRINT(...)                                                       \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
  } while (false)
#else
#define DEBUG_PRINT(...)                                                       \
  do {                                                                         \
  } while (false)
#endif

static char args_doc[] = "FILE [FILES...]";

struct arguments {
  int num_processes = 4;
  char *file; // need at least 1 file
  char **files;
};

/// Parse a single option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = (struct arguments *)state->input;
  switch (key) {
  case 'n':
    args->num_processes = std::atoi(arg);
    break;
  case ARGP_KEY_NO_ARGS:
    argp_usage(state);
  case ARGP_KEY_ARG:
    args->file = arg;
    args->files = &state->argv[state->next];
    state->next = state->argc;
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  struct arguments args;
  struct argp_option options[] = {
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."}, {0}};
  struct argp argp = {options, parse_opt, args_doc, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &args);

  if (status) {
    fprintf(stderr, "Failed to parse arguments, error code: %d", status);
    exit(status);
  }

  DEBUG_PRINT("Number of processes: %d\n", args.num_processes);
  DEBUG_PRINT("Input files: %s", args.file);
  for (int i = 0; args.files[i]; ++i) {
    DEBUG_PRINT(", %s", args.files[i]);
  }
  DEBUG_PRINT("\n");

  // Put all files into a vector
  std::vector<std::string> files;
  files.push_back(args.file);
  for (int i = 0; args.files[i]; ++i) {
    files.push_back(args.files[i]);
  }

  // Read data from files
  std::vector<int64_t> data;
  for (const auto &f : files) {
    std::ifstream infile(f);
    std::istream_iterator<int64_t> input(infile);
    std::copy(input, std::istream_iterator<int64_t>(),
              std::back_inserter(data));
  }
  DEBUG_PRINT("Number of integers: %zu\n", data.size());

  return 0;
}
