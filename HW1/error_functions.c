#include "error_functions.h"

#include <sys/types.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "ename.c.inc"

typedef enum { FALSE, TRUE } Boolean;

#ifdef __GNUC__
__attribute__((__noreturn__))
#endif
static void
terminate(Boolean useExit3) {
  if (useExit3)
    exit(EXIT_FAILURE);
  else
    _exit(EXIT_FAILURE);
}

static void outputError(Boolean useErr, int err, Boolean flushStdout,
                        const char *format, va_list ap) {
#define BUF_SIZE 500
  char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];
  vsnprintf(userMsg, BUF_SIZE, format, ap);
  if (useErr)
    snprintf(errText, BUF_SIZE, " [%s %s]",
             (err > 0 && err <= MAX_ENAME) ? ename[err] : "?UNKNOWN?",
             strerror(err));
  else
    snprintf(errText, BUF_SIZE, ":");
  snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);
  if (flushStdout)
    fflush(stdout); /* Flush any pending stdout */
  fputs(buf, stderr);
  fflush(stderr); /* In case stderr is not line-buffered */
}

void errMsg(const char *format, ...) {
  va_list argList;
  int savedErrno;
  savedErrno = errno; /* In case we change it here */
  va_start(argList, format);
  outputError(TRUE, errno, TRUE, format, argList);
  va_end(argList);
  errno = savedErrno;
}

void errExit(const char *format, ...) {
  va_list argList;
  va_start(argList, format);
  outputError(TRUE, errno, TRUE, format, argList);
  va_end(argList);
  terminate(TRUE);
}

void err_exit(const char *format, ...) {
  va_list argList;
  va_start(argList, format);
  outputError(TRUE, errno, FALSE, format, argList);
  va_end(argList);
  terminate(FALSE);
}

void errExitEN(int errnum, const char *format, ...) {
  va_list argList;
  va_start(argList, format);
  outputError(TRUE, errnum, TRUE, format, argList);
  va_end(argList);
  terminate(TRUE);
}

void fatal(const char *format, ...) {
  va_list argList;
  va_start(argList, format);
  outputError(FALSE, 0, TRUE, format, argList);
  va_end(argList);
  terminate(TRUE);
}
