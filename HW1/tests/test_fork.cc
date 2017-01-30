#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <string.h>

int main(int argc, char **argv) {
  int fdes[2];
  pid_t pid;

  std::vector<std::string> data = {"a", "abc", "abcdef"};

  if (pipe(fdes) == -1) {
    fprintf(stderr, "Failed to open pipe");
  }

  if ((pid = fork()) < 0) {
    exit(1);
  } else if (pid == 0) {
    printf("child\n");
    // close write end
    if (close(fdes[1]) == -1) {
      fprintf(stderr, "close write failed");
      exit(1);
    }

    FILE *fr = fdopen(fdes[0], "r");
    if (!fr) {
      fprintf(stderr, "error");
    }

    std::vector<std::string> sub_data;

    char buffer[16];
    memset(&buffer[0], 0, sizeof(buffer));
    while(!feof(fr)) {
      while (fgets(buffer, 16, fr) != NULL) {
        size_t len = strlen(buffer);
        printf("len %zu\n", len);
        if ((len > 0) && (buffer[len - 1] == '\n')) break;
      }
    }

    fclose(fr);

    exit(0);
  } else {
    printf("parent\n");
    // close read end
    if (close(fdes[0]) == -1) {
      fprintf(stderr, "close read failed");
      exit(1);
    }

    FILE *fw = fdopen(fdes[1], "w");
    if (!fw) {
      fprintf(stderr, "error");
    }

    for (const auto &d : data) {
      fprintf(fw, "%s\n", d.c_str());
    }

    // close write end
    fclose(fw);
  }

  return 0;
}
