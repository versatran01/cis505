#include <argp.h>

#include <string>
#include <utility>
#include <vector>

static char args_doc[] = "FILE [FILES...]";

struct arguments {
  int num_processes = 4;
  char *file; // need at least 1 file
  char **files;
};

/// Parse a single option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = (struct arguments *)state->input;
  switch (key) {
  case 'n':
    arguments->num_processes = atoi(arg);
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
  struct arguments arguments;
  struct argp_option options[] = {
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."}, {0}};
  struct argp argp = {options, parse_opt, args_doc, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &arguments);
  printf("Number of processes: %d\n", arguments.num_processes);
  printf("Files: %s", arguments.file);
  for (int i = 0; arguments.files[i]; ++i) {
    printf(", %s", arguments.files[i]);
  }
  return status;
}
