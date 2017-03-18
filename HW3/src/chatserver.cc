#include <cstdio>
#include <cstdlib>

#define LOGURU_IMPLEMENTATION 1
#include "loguru.hpp"

#include "cmdparser.hpp"

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "*** Author: Chao Qu (quchao)\n");
    exit(1);
  }

  return 0;
}
