#include "mysort.h" // bubble_sort, divide_equal, merge_sort
#include "common.h" // errExit, fatal

#include <argp.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

using data_t = long long;

enum { READ, WRITE };

static char args_doc[] = "FILE [FILES...]";

/**
 * @brief The arguments struct
 */
struct argp_args {
  int num_processes = 4;
  int use_threads = 0;
  int verbose = 0; // verbose mode
  char *file;      // need at least 1 file
  char **files;
};

/**
 * @brief Parse command line options, used by argp
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct argp_args *args = (struct argp_args *)state->input;
  switch (key) {
  case 'n':
    args->num_processes = std::atoi(arg);
    break;
  case 't':
    args->use_threads = 1;
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
 * @brief Parse command line arguments
 * @return argp_args struct that contains all the options and inputs
 */
argp_args ParseCmdArguments(int argc, char **argv) {
  // Parse command line arguments
  struct argp_args args;
  struct argp_option options[] = {
      {0, 'n', "NUM_PROCESSES", 0, "Number of processes."},
      {0, 't', 0, 0, "Use threads instead of processes."},
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

  return args;
}

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
 * @brief The ThreadArgs struct
 */
struct PthreadArgs {
  int id;
  data_t *first;
  data_t *last;
};

/**
 * @brief SortWorker
 * @param args
 * @return
 */
void *SortWorker(void *args) {
  const PthreadArgs *thread_args = (const PthreadArgs *)args;
  DEBUG_PRINT("Thread %d", thread_args->id);

  BubbleSort(thread_args->first, thread_args->last);
  pthread_exit(NULL);
}

/**
 * @brief Extract filenames from arpg_args struct
 * @return a vector of filenames to read data from
 */
std::vector<std::string> ExtractFilesFromArgs(const argp_args &args) {
  // Put all files into a vector of strings
  std::vector<std::string> files;
  files.push_back(args.file);
  for (int i = 0; args.files[i]; ++i) {
    files.push_back(args.files[i]);
  }

  return files;
}

/**
 * @brief ReadRangeFromFile
 */
void ReadRangeFromFile(data_t *first, data_t *last, FILE *file) {
  while (!feof(file)) {
    const auto res = fscanf(file, "%lld\n", first);
    if (res == 1)
      ++first;
  }
  assert(first == last && "Read data mismatch");
}

/**
 * @brief WriteRangeToFile
 */
void WriteRangeToFile(data_t *first, data_t *last, FILE *file) {
  while (first != last) {
    fprintf(file, "%lld\n", *first++);
  }
}

/**
 * @brief A multi-threaded sort
 * @param data will be modified
 * @param split
 */
template <typename T>
void SortMultiThread(std::vector<T> &data, const std::vector<size_t> &split,
                     int n_threads) {
  std::vector<pthread_t> threads(n_threads);
  std::vector<PthreadArgs> threads_args(n_threads);
  for (int i = 0; i < n_threads; ++i) {
    // Create a thread
    threads_args[i].id = i;
    threads_args[i].first = &data[split[i]];
    threads_args[i].last = &data[split[i + 1]];
    pthread_create(&threads[i], NULL, SortWorker, &threads_args[i]);
  }

  for (auto &thread : threads) {
    const auto s = pthread_join(thread, NULL);
    if (s != 0)
      errExitEN(s, "pthread join");
  }
}

void SortMultiProcess();

/**
 * @brief main
 */
int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto args = ParseCmdArguments(argc, argv);

  // TODO: this is for testing
  //  args.num_processes = 2;
  //  args.use_threads = 1;

  DEBUG_PRINT("Number of processes: %d\n", args.num_processes);
  DEBUG_PRINT("Use threads: %d\n", args.use_threads);
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

  // TODO: this is for testing
  //  std::vector<data_t> data = {7, 6, 5, 4, 3, 2, 1, 0};

  auto data = ReadDataFromFiles<data_t>(files);
  DEBUG_PRINT("Number of data: %zu\n", data.size());

  // If there's no data to sort, just exit
  if (data.empty())
    exit(EXIT_SUCCESS);

  // ====== Special case ======
  // single process or thread
  // or when the number to data to process <= num_processes
  // just use bubble_sort on the entire data
  if (args.num_processes == 1 ||
      data.size() <= static_cast<size_t>(args.num_processes)) {
    BubbleSort(data.begin(), data.end());

    PrintRangeToStdout(data.cbegin(), data.cend());
    exit(EXIT_SUCCESS);
  }

  // Split data into n almost equal parts
  const auto split = DivideEqual(data.size(), args.num_processes);
  DEBUG_PRINT("Split: ");
  for (const auto &s : split) {
    DEBUG_PRINT("%zu ", s);
  }
  DEBUG_PRINT("\n");

  // ====== Common case: thread ======
  // multi threads
  if (args.use_threads) {
    // Multi-thread sort
    SortMultiThread(data, split, args.num_processes);
    // Merge sorted data
    const auto merged = MergeSort(data, split);
    // Print to stdout
    PrintRangeToStdout(merged.cbegin(), merged.cend());

    exit(EXIT_SUCCESS);
  }

  // ====== Common case: process =======
  // multiple processes

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

      ReadRangeFromFile(&sub_data[0], &sub_data[length], fp2c_r);

      // Close read end
      fclose(fp2c_r);

      // Sort sub_data
      BubbleSort(sub_data.begin(), sub_data.end());

      // Child write data back to parent
      FILE *fc2p_w = fdopen(child.c2p[WRITE], "w");
      if (fc2p_w == NULL) {
        errExit("[C%d] fdopen c2p write end failed.", i);
      }

      WriteRangeToFile(&sub_data[0], &sub_data[length], fc2p_w);

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
      WriteRangeToFile(&data[split[i]], &data[split[i + 1]], fp2c_w);

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

    ReadRangeFromFile(&data[split[i]], &data[split[i + 1]], fc2p_r);

    fclose(fc2p_r);

    int status;
    waitpid(child.pid, &status, 0);
    DEBUG_PRINT("[P] child %d exit with status %d.\n", i, status);
  }

  // Do merge sort here and print to stdout
  const auto merged = MergeSort(data, split);
  PrintRangeToStdout(merged.cbegin(), merged.cend());

  return 0;
}
