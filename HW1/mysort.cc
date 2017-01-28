#include "mysort.h" // bubble_sort, divide_equal, merge_sort

#include <argp.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <string>

using data_t = long long;

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

#define PIPE_OPEN_FAILURE 2
#define PIPE_CLOSE_FAILURE 3
#define FDOPEN_FAILURE 4

static char args_doc[] = "FILE [FILES...]";

struct arguments {
  int num_processes = 4;
  int use_thread = 0;
  int verbose = 0; // verbose mode
  char *file;      // need at least 1 file
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
  case 'v':
    args->verbose = 1;
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
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."},
      {0, 't', 0, 0, "Use threads instead of processes."},
      {0, 'v', 0, 0, "Verbose mode."},
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
    exit(EXIT_FAILURE);
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
  // TODO: change back after testing
  std::vector<data_t> data = {4, 3, 2, 1, 8, 7, 6, 5};
  args.num_processes = 2;
  //  for (const auto &f : files) {
  //    std::ifstream infile(f);
  //    std::istream_iterator<data_t> input(infile);
  //    std::copy(input, std::istream_iterator<data_t>(),
  //    std::back_inserter(data));
  //  }
  DEBUG_PRINT("Number of integers: %zu\n", data.size());

  if (data.empty()) {
    fprintf(stderr, "No data to sort");
    exit(EXIT_FAILURE);
  }

  // Special case
  // single process or thread
  // or when the number to data to process <= num_processes
  if (args.num_processes == 1 || data.size() <= args.num_processes) {
    bubble_sort(data.begin(), data.end());

    std::copy(data.begin(), data.end(),
              std::ostream_iterator<data_t>(std::cout, "\n"));
    exit(EXIT_SUCCESS);
  }

  // Divide array into N almost equal parts
  const auto split = divide_equal(data.size(), args.num_processes);

  /**
   * @brief The Child struct
   */
  struct Child {
    pid_t pid;
    int p2c[2]; // pipe from parent to child
    int c2p[2]; // pipe from child to parent
  };

  std::vector<Child> children(args.num_processes);

  // Common case, multiple processes
  for (int i = 0; i < args.num_processes; ++i) {
    Child &child = children[i];

    // Create a pair of pipes for each child before fork
    if (pipe(child.p2c) == -1) {
      fprintf(stderr, "Create pipe p2c failed\n");
      exit(PIPE_OPEN_FAILURE);
    }

    if (pipe(child.c2p) == -1) {
      fprintf(stderr, "Create pipe c2p failed\n");
      exit(PIPE_OPEN_FAILURE);
    }

    // Fork a child process
    if ((child.pid = fork()) < 0) {
      fprintf(stderr, "Fork failed\n");
      exit(EXIT_FAILURE);
    } else if (child.pid == 0) {
      // Child
      DEBUG_PRINT("Child %d: %d\n", i, (int)getpid());

      // Close write end of p2c
      if (close(child.p2c[1]) == -1) {
        fprintf(stderr, "Close write end of p2c pipe failed\n");
        exit(PIPE_CLOSE_FAILURE);
      }

      // Close read end of c2p
      if (close(child.c2p[0]) == -1) {
        fprintf(stderr, "Close read end of c2p pipe failed\n");
        exit(PIPE_CLOSE_FAILURE);
      }

      // Child read data from parent
      // first convert pipe file handle to FILE object
      FILE *fp2c_r = fdopen(child.p2c[0], "r");
      if (fp2c_r == NULL) {
        fprintf(stderr, "fdopen p2c read failed\n");
        exit(FDOPEN_FAILURE);
      }

      // Read data from parent
      const auto length = split[i + 1] - split[i];
      std::vector<data_t> sub_data(length);

      size_t j = 0;
      while (!feof(fp2c_r)) {
        fscanf(fp2c_r, "%lld\n", &sub_data[j++]);
      }

      if (j != length) {
        fprintf(stderr, "Data mismatch, expected: %zu, actual: %zu", length, j);
        exit(EXIT_FAILURE);
      }

      // Close read end
      fclose(fp2c_r);

      // Sort sub_data
      bubble_sort(sub_data.begin(), sub_data.end());

      // Write data back to parent
      FILE *fc2p_w = fdopen(child.c2p[1], "w");
      if (fc2p_w == NULL) {
        fprintf(stderr, "fdopen c2p write failed\n");
        exit(FDOPEN_FAILURE);
      }

      for (const auto &d : sub_data) {
        fprintf(fc2p_w, "%lld\n", d);
      }

      // Close write end
      fclose(fc2p_w);

      exit(EXIT_SUCCESS);
    } else {
      // Parent
      DEBUG_PRINT("Parent: %d\n", (int)getpid());

      // Close read end of p2c
      if (close(child.p2c[0]) == -1) {
        fprintf(stderr, "Close read end of p2c pipe failed\n");
        exit(PIPE_CLOSE_FAILURE);
      }

      // Close write end of c2p
      if (close(child.c2p[1]) == -1) {
        fprintf(stderr, "Close write end of c2p pipe failed\n");
        exit(PIPE_CLOSE_FAILURE);
      }

      // Parent write to data to child
      // First convert pipe file handle to file object
      FILE *fp2c_w = fdopen(child.p2c[1], "w");
      if (fp2c_w == NULL) {
        fprintf(stderr, "fdopen p2c write failed\n");
        exit(FDOPEN_FAILURE);
      }

      // Write data to child
      auto first = split[i];
      const auto last = split[i + 1];
      const auto length = last - first;
      DEBUG_PRINT("Write to child %d, length %zu\n", i, length);

      for (; first != last; ++first) {
        fprintf(fp2c_w, "%lld\n", data[first]);
      }

      // Close write end
      fclose(fp2c_w);
    }
  }

  // Parent start reading from pipe
  for (int i = 0; i < args.num_processes; ++i) {
    Child &child = children[i];

    // Start reading back from child
    auto first = split[i];
    const auto last = split[i + 1];
    const auto length = last - first;

    FILE *fc2p_r = fdopen(child.c2p[0], "r");
    if (fc2p_r == NULL) {
      fprintf(stderr, "fdopen c2p read failed\n");
      exit(FDOPEN_FAILURE);
    }

    DEBUG_PRINT("Child %d", i);
    while (!feof(fc2p_r)) {
      data_t d;
      fscanf(fc2p_r, "%lld\n", &d);
      DEBUG_PRINT("%lld ", d);
    }
    DEBUG_PRINT("\n");

    fclose(fc2p_r);

    int status;
    waitpid(child.pid, &status, 0);
    DEBUG_PRINT("child %d finished with status %d", i, status);
  }

  return 0;
}
