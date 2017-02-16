#ifndef LPI_H
#define LPI_H

/// These functions are taken from LINUX PROGRAMMING INTERFACES 3RD EDITION
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif

#define DEBUG 1
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

/**
 * @brief errMsg prints a message on standard error
 */
void errMsg(const char *format, ...);

/**
 * @brief errExit Like errMsg, but terminates the program, by calling exit()
 */
void errExit(const char *format, ...) NORETURN;

/**
 * @brief err_exit Similar to errExit(), doesn't flush stdout before printing,
 * Terminates process by calling _exit() instead of exit()
 */
void err_exit(const char *format, ...) NORETURN;

/**
 * @brief errExitEN Similar to errExit(), prints the text corresponding to the
 * error number given in the argument errnum
 */
void errExitEN(int errnum, const char *format, ...) NORETURN;

/**
 * @brief fatal Used to diagnose general errors, terminates program
 */
void fatal(const char *format, ...) NORETURN;

/**
 * @brief cmdLineErr Diagnose errors in command-line argument usage
 */
void usageErr(const char *format, ...) NORETURN;

/**
 * @brief cmdLineErr Similar to usageErr(), but is intended for diagnosing
 * errors in the command-line arguments specified to a program
 */
void cmdLineErr(const char *format, ...) NORETURN;

#endif // LPI_H
