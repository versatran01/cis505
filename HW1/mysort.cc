#include <argp.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <string>
#include <unistd.h>
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
  std::vector<data_t> data = {1, 2, 3, 4, 5, 6, 7, 8};
  //  for (const auto &f : files) {
  //    std::ifstream infile(f);
  //    std::istream_iterator<data_t> input(infile);
  //    std::copy(input, std::istream_iterator<data_t>(),
  //    std::back_inserter(data));
  //  }
  DEBUG_PRINT("Number of integers: %zu\n", data.size());

  if (data.empty()) {
    fprintf(stderr, "No data to sort");
    exit(1);
  }

  // Special case, single process or thread
  // TODO: also when the number of data to be sorted is less than num_processes
  if (args.num_processes == 1) {
    // TODO: mergesort or bubblesort?
    bubble_sort(data.begin(), data.end());
    // write to stdout
    std::copy(data.begin(), data.end(),
              std::ostream_iterator<data_t>(std::cout, "\n"));
    exit(0);
  }

  /**
   * @brief The Child struct
   */
  struct Child {
    pid_t pid;
    std::array<int, 2> p2c; // pipe from parent to child
    std::array<int, 2> c2p; // pipe from child to parent
  };

  const int N = args.num_processes;
  std::vector<Child> children(N);

  // Common case, multiple processes
  for (int i = 0; i < N; ++i) {
    Child &child = children[i];

    // Create a pair of pipes for each child before fork
    if (pipe(child.p2c.data()) == -1) {
      fprintf(stderr, "Create pipe p2c failed\n");
      exit(1);
    }

    if (pipe(child.c2p.data()) == -1) {
      fprintf(stderr, "Create pipe c2p failed\n");
      exit(1);
    }

    // Fork a child process
    if ((child.pid = fork()) < 0) {
      fprintf(stderr, "Fork failed\n");
      exit(1);
    } else if (child.pid == 0) {
      // Child
      DEBUG_PRINT("Child %d: %d\n", i, (int)getpid());

      // Close write end of p2c
      if (close(child.p2c[1]) == -1) {
        fprintf(stderr, "Close write end of p2c pipe failed\n");
        exit(1);
      }

      // Close read end of c2p
      if (close(child.c2p[0]) == -1) {
        fprintf(stderr, "Close read end of c2p pipe failed\n");
        exit(1);
      }

      // Child read data from parent
      // first convert pipe file handle to FILE object
      FILE *fp2c_r = fdopen(child.p2c[0], "r");
      if (fp2c_r == NULL) {
        fprintf(stderr, "fdopen p2c read failed\n");
        exit(1);
      }

      fclose(fp2c_r);

      // Sort the array

      // Child write data back to parent
      FILE *fc2p_w = fdopen(child.c2p[1], "w");
      if (fc2p_w == NULL) {
        fprintf(stderr, "fdopen c2p write failed\n");
        exit(1);
      }

      // TODO: write data to parent
      fclose(fc2p_w);

      exit(0);
    } else {
      // Parent
      DEBUG_PRINT("Parent: %d\n", (int)getpid());

      // Close read end of p2c
      if (close(child.p2c[0]) == -1) {
        fprintf(stderr, "Close read end of p2c pipe failed\n");
        exit(1);
      }

      // Close write end of c2p
      if (close(child.c2p[1]) == -1) {
        fprintf(stderr, "Close write end of c2p pipe failed\n");
        exit(1);
      }

      // Parent write to data to child
      // First convert pipe file handle to file object
      FILE *fp2c_w = fdopen(child.p2c[1], "w");
      if (fp2c_w == NULL) {
        fprintf(stderr, "fdopen p2c write failed\n");
        exit(1);
      }

      // TODO: write data to child
      // Looks like we don't need this?
      // fclose(fp2c_w);

      // Close write end of p2c when writing is done
      if (close(child.p2c[1]) == -1) {
        fprintf(stderr, "Close write end of p2c pipe failed\n");
        exit(1);
      }
    }
  }

  // Parent wait for children

  return 0;
}
