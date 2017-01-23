#include <argp.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <string>
#include <vector>

using data_t = int64_t;

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
  int use_thread = 0;
  char *file; // need at least 1 file
  char **files;
};

/**
 * @brief parse_opt
 * @param key
 * @param arg
 * @param state
 * @return error code
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = (struct arguments *)state->input;
  switch (key) {
  case 'n':
    args->num_processes = std::atoi(arg);
    break;
  case 't':
    args->use_thread = 1;
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

/**
 * @brief bubble_sort
 * @param first Iterator to the first element in range
 * @param last Iterator to the last element in range
 * @param compare Functor that follows strict weak ordering
 */
template <typename Iter, typename Comp = std::less<
                             typename std::iterator_traits<Iter>::value_type>>
void bubble_sort(Iter first, Iter last, Comp compare = Comp()) {
  // Check if Iter is random access iterator, fail at compile time if it is not
  using Iter_category = typename std::iterator_traits<Iter>::iterator_category;
  static_assert(
      std::is_same<std::random_access_iterator_tag, Iter_category>::value,
      "bubble_sort: Iter should be random access iterators or pointers to an "
      "array");

  // sort routine
  Iter i, j;
  for (i = first; i != last; ++i)
    for (j = first; j < i; ++j)
      if (compare(*i, *j))
        std::iter_swap(i, j);
}

int main(int argc, char *argv[]) {
  // Parse command line arguments
  struct arguments args;
  struct argp_option options[] = {
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."},
      {0, 't', 0, 0, "Use threads instead of processes."},
      {0}};
  struct argp argp = {options, parse_opt, args_doc, 0};
  int status = argp_parse(&argp, argc, argv, 0, 0, &args);

  if (status) {
    fprintf(stderr, "Failed to parse arguments, error code: %d\n", status);
    exit(status);
  }

  if (args.num_processes <= 0) {
    fprintf(stderr, "Number of processes is not a positive integer: %d\n",
            args.num_processes);
    exit(1);
  }

  DEBUG_PRINT("Number of processes: %d\n", args.num_processes);
  DEBUG_PRINT("Use threads: %d\n", args.use_thread);
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
  std::vector<data_t> data;
  for (const auto &f : files) {
    std::ifstream infile(f);
    std::istream_iterator<data_t> input(infile);
    std::copy(input, std::istream_iterator<data_t>(), std::back_inserter(data));
  }
  DEBUG_PRINT("Number of integers: %zu\n", data.size());

  // Special case, single process or thread
  if (args.num_processes == 1) {
    // TODO: mergesort or bubblesort?
    bubble_sort(data.begin(), data.end());
    // write to stdout
    std::copy(data.begin(), data.end(),
              std::ostream_iterator<data_t>(std::cout, "\n"));
    exit(0);
  }

  // Common case, multiple processes or threads

  return 0;
}
