#include "mysort.h" // bubble_sort, divide_equal, merge_sort
#include "common.h" // errExit, fatal

#include <argp.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <string>

using data_t = long long;

enum { READ, WRITE };

static char args_doc[] = "FILE [FILES...]";

/**
 * @brief The Child struct
 */
struct Child {
  int id;
  pid_t pid;
  int p2c[2]; // pipe from parent to child
  int c2p[2]; // pipe from child to parent
};

/**
 * @brief The arguments struct
 */
struct arguments {
  int num_processes = 4;
  int use_thread = 0;
  int verbose = 0; // verbose mode
  char *file;      // need at least 1 file
  char **files;
};

/**
 * @brief Parse command line options
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

/**
 * @brief Extract filenames from command line arguments
 * @param args
 * @return
 */
std::vector<std::string> ExtractFilesFromArgs(const arguments &args) {
  // Put all files into a vector of strings
  std::vector<std::string> files;
  files.push_back(args.file);
  for (int i = 0; args.files[i]; ++i) {
    files.push_back(args.files[i]);
  }

  return files;
}

/**
 * @brief Combine data from files into a vector
 * @param files
 * @todo Make data_t a template parameter?
 * @return
 */
std::vector<data_t> ReadDataFromFiles(const std::vector<std::string> &files) {
  // Read data from files
  std::vector<data_t> data;
  for (const auto &f : files) {
    std::ifstream infile(f);
    std::istream_iterator<data_t> input(infile);
    std::copy(input, std::istream_iterator<data_t>(), std::back_inserter(data));
  }

  return data;
}

void WriteRangeToPipe() {}

void ReadPipeToRange() {}

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
    fatal("Failed to parse arguments, error code: %d.", status);
  }

  if (args.num_processes <= 0) {
    cmdLineErr("Number of processes is not a positive integer: %d.",
               args.num_processes);
  }

  // TODO: change this back
  args.num_processes = 3;

  DEBUG_PRINT("Number of processes: %d\n", args.num_processes);
  DEBUG_PRINT("Use threads: %d\n", args.use_thread);
  DEBUG_PRINT("Input files: %s", args.file);
  for (int i = 0; args.files[i]; ++i) {
    DEBUG_PRINT(", %s", args.files[i]);
  }
  DEBUG_PRINT("\n");

  // Process command line arguments
  const auto files = ExtractFilesFromArgs(args);
  DEBUG_PRINT("Read data from: ");
  for (const auto &f : files) {
    DEBUG_PRINT("%s ", f.c_str());
  }
  DEBUG_PRINT("\n");

  // TODO: change this back
  //  auto data = ReadDataFromFiles(files);
  std::vector<data_t> data = {8, 7, 6, 5, 4, 3, 2, 1};
  DEBUG_PRINT("Number of integers: %zu\n", data.size());

  // If there's no data to sort, just exit
  if (data.empty())
    exit(EXIT_SUCCESS);

  // ====== Special case ======
  // single process or thread
  // or when the number to data to process <= num_processes
  // just use bubble_sort on the entire data
  if (args.num_processes == 1 || data.size() <= args.num_processes) {
    bubble_sort(data.begin(), data.end());

    std::copy(data.begin(), data.end(),
              std::ostream_iterator<data_t>(std::cout, "\n"));
    exit(EXIT_SUCCESS);
  }

  // ====== Common case =======
  // multiple processes
  // Divide array into N almost equal parts
  const auto split = divide_equal(data.size(), args.num_processes);

  std::vector<Child> children(args.num_processes);

  for (int i = 0; i < args.num_processes; ++i) {
    Child &child = children[i];
    child.id = i;

    // Create a pair of pipes for each child before fork
    if (pipe(child.p2c) == -1) {
      errExit("[P] Create pipe p2c failed for child %d.", i);
    }

    if (pipe(child.c2p) == -1) {
      errExit("[P] Create pipe c2p failed for child %d.", i);
    }

    // Fork a child process
    if ((child.pid = fork()) < 0) {
      errExit("[P] Fork failed for child %d.", i);
    } else if (child.pid == 0) {
      // Child
      DEBUG_PRINT("Child %d, pid %d\n", i, (int)getpid());

      // For child, we close the write end of p2c and read end of c2p
      if (close(child.p2c[WRITE]) == -1) {
        errExit("[C%d] Close write end of p2c pipe failed.", i);
      }

      if (close(child.c2p[READ]) == -1) {
        errExit("[C%d] Close read end of c2p pipe failed.", i);
      }

      // Child read data from parent
      FILE *fp2c_r = fdopen(child.p2c[READ], "r");
      if (fp2c_r == NULL) {
        errExit("[C%d] fdopen p2c read end failed.", i);
      }

      const auto length = split[i + 1] - split[i];
      // This allocation might be wasteful since data will be overriden
      std::vector<data_t> sub_data(length);

      size_t j = 0;
      while (!feof(fp2c_r)) {
        fscanf(fp2c_r, "%lld\n", &sub_data[j++]);
      }

      if (j != length) {
        fatal("[C%d] Data mismatch, expected: %zu, actual: %zu.", length, j);
      }
      DEBUG_PRINT("[C%d] Read %zu data.\n", i, length);

      // Close read end
      fclose(fp2c_r);

      // Sort sub_data
      bubble_sort(sub_data.begin(), sub_data.end());

      // Child write data back to parent
      FILE *fc2p_w = fdopen(child.c2p[WRITE], "w");
      if (fc2p_w == NULL) {
        errExit("[C%d] fdopen c2p write end failed.", i);
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

      // For child, we close the read end of p2c and write end of c2p
      if (close(child.p2c[READ]) == -1) {
        errExit("[C%d] Close read end of p2c pipe failed.", i);
      }

      if (close(child.c2p[WRITE]) == -1) {
        errExit("[C%d] Close write end of c2p pipe failed.", i);
      }

      // Parent write to data to child
      // First convert pipe file handle to file object
      FILE *fp2c_w = fdopen(child.p2c[WRITE], "w");
      if (fp2c_w == NULL) {
        errExit("[C%d] fdopen p2c write end failed.", i);
      }

      // Write data to child
      auto first = split[i];
      const auto last = split[i + 1];
      const auto length = last - first;
      DEBUG_PRINT("[P] Write to child %d, length %zu\n", i, length);

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
    FILE *fc2p_r = fdopen(child.c2p[0], "r");
    if (fc2p_r == NULL) {
      errExit("[P] fdopen c2p read end failed.");
    }

    auto first = split[i];
    const auto last = split[i + 1];
    const auto length = last - first;

    while (!feof(fc2p_r)) {
      fscanf(fc2p_r, "%lld\n", &data[first++]);
    }

    // Check if we read the correct amount of data
    if (first != last) {
      fatal("[P] Data mismatch from child %i, expected: %zu, actual: %zu.", i,
            length, first + length - last);
    }
    DEBUG_PRINT("[P] Read %zu data from child %d.\n", length, i);

    fclose(fc2p_r);

    int status;
    waitpid(child.pid, &status, 0);
    DEBUG_PRINT("[P] child %d exit with status %d.\n", i, status);
  }

  // Print all data
  for (const auto &d : data) {
    DEBUG_PRINT("%lld ", d);
  }
  DEBUG_PRINT("\n");

  // Do merge sort here

  return 0;
}
