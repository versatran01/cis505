#include "lpi.h"

#include <cstring>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>

// clang-format off
static const char *ename[] = {
  /*  0 */ "",
  /*  1 */ "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO", "E2BIG",
  /*  8 */ "ENOEXEC", "EBADF", "ECHILD", "EAGAIN/EWOULDBLOCK", "ENOMEM",
  /*  13 */ "EACCES", "EFAULT", "ENOTBLK", "EBUSY", "EEXIST", "EXDEV",
  /*  19 */ "ENODEV", "ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE",
  /*  25 */ "ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE", "EROFS",
  /*  31 */ "EMLINK", "EPIPE", "EDOM", "ERANGE", "EDEADLK/EDEADLOCK",
  /*  36 */ "ENAMETOOLONG", "ENOLCK", "ENOSYS", "ENOTEMPTY", "ELOOP", "",
  /*  42 */ "ENOMSG", "EIDRM", "ECHRNG", "EL2NSYNC", "EL3HLT", "EL3RST",
  /*  48 */ "ELNRNG", "EUNATCH", "ENOCSI", "EL2HLT", "EBADE", "EBADR",
  /*  54 */ "EXFULL", "ENOANO", "EBADRQC", "EBADSLT", "", "EBFONT", "ENOSTR",
  /*  61 */ "ENODATA", "ETIME", "ENOSR", "ENONET", "ENOPKG", "EREMOTE",
  /*  67 */ "ENOLINK", "EADV", "ESRMNT", "ECOMM", "EPROTO", "EMULTIHOP",
  /*  73 */ "EDOTDOT", "EBADMSG", "EOVERFLOW", "ENOTUNIQ", "EBADFD",
  /*  78 */ "EREMCHG", "ELIBACC", "ELIBBAD", "ELIBSCN", "ELIBMAX",
  /*  83 */ "ELIBEXEC", "EILSEQ", "ERESTART", "ESTRPIPE", "EUSERS",
  /*  88 */ "ENOTSOCK", "EDESTADDRREQ", "EMSGSIZE", "EPROTOTYPE",
  /*  92 */ "ENOPROTOOPT", "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
  /*  95 */ "EOPNOTSUPP/ENOTSUP", "EPFNOSUPPORT", "EAFNOSUPPORT",
  /*  98 */ "EADDRINUSE", "EADDRNOTAVAIL", "ENETDOWN", "ENETUNREACH",
  /*  102 */ "ENETRESET", "ECONNABORTED", "ECONNRESET", "ENOBUFS", "EISCONN",
  /*  107 */ "ENOTCONN", "ESHUTDOWN", "ETOOMANYREFS", "ETIMEDOUT",
  /*  111 */ "ECONNREFUSED", "EHOSTDOWN", "EHOSTUNREACH", "EALREADY",
  /*  115 */ "EINPROGRESS", "ESTALE", "EUCLEAN", "ENOTNAM", "ENAVAIL",
  /*  120 */ "EISNAM", "EREMOTEIO", "EDQUOT", "ENOMEDIUM", "EMEDIUMTYPE",
  /*  125 */ "ECANCELED", "ENOKEY", "EKEYEXPIRED", "EKEYREVOKED",
  /*  129 */ "EKEYREJECTED", "EOWNERDEAD", "ENOTRECOVERABLE", "ERFKILL"
};
// clang-format on

#define MAX_ENAME 132

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

void usageErr(const char *format, ...) {
  va_list argList;
  fflush(stdout); /* Flush any pending stdout */
  fprintf(stderr, "Usage: ");
  va_start(argList, format);
  vfprintf(stderr, format, argList);
  va_end(argList);
  fflush(stderr); /* In case stderr is not line-buffered */
  exit(EXIT_FAILURE);
}

void cmdLineErr(const char *format, ...) {
  va_list argList;
  fflush(stdout); /* Flush any pending stdout */
  fprintf(stderr, "Command-line usage error: ");
  va_start(argList, format);
  vfprintf(stderr, format, argList);
  va_end(argList);
  fflush(stderr); /* In case stderr is not line-buffered */
  exit(EXIT_FAILURE);
}
