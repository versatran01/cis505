#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

int main(int argc, char **argv) {
  int fdes[2];
  pid_t pid;

  std::vector<int> data = {1, 2, 3, 4, 5};

  if (pipe(fdes) == -1) {
    fprintf(stderr, "Failed to open pipe");
  }

  if ((pid = fork()) < 0) {
    exit(1);
  } else if (pid == 0) {
    printf("child\n");
    // close write end
    close(fdes[1]);

    FILE *fr = fdopen(fdes[0], "r");
    if (!fr) {
      fprintf(stderr, "error");
    }

    std::vector<int> sub_data(data.size());

    int j = 0;
    while (!feof(fr)) {
      fscanf(fr, "%d\n", &sub_data[j++]);
    }
    printf("j: %d\n", j);

    for (const auto &d : sub_data) {
      printf("%d ", d);
    }
    printf("\n");

    fclose(fr);
    close(fdes[0]);

    exit(0);
  } else {
    printf("parent\n");
    // close read end
    close(fdes[0]);

    FILE *fw = fdopen(fdes[1], "w");
    if (!fw) {
      fprintf(stderr, "error");
    }

    for (const auto &d : data) {
      fprintf(fw, "%d\n", d);
    }

    // close write end
    fclose(fw);
    close(fdes[1]);
  }

  return 0;
}
