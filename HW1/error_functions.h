#ifndef ERROR_FUNCTIONS_H
#define ERROR_FUNCTIONS_H

#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif

/**
 * @brief errMsg prints a message on standard error
 * @param format
 */
void errMsg(const char *format, ...);

/**
 * @brief errExit Like errMsg, but terminates the program, by calling exit()
 * @param format
 */
void errExit(const char *format, ...) NORETURN;

/**
 * @brief err_exit Similar to errExit(), doesn't flush stdout before printing,
 * Terminates process by calling _exit() instead of exit()
 * @param format
 */
void err_exit(const char *format, ...) NORETURN;

/**
 * @brief errExitEN Similar to errExit(), prints the text corresponding to the
 * error number given in the argument errnum
 * @param errnum
 * @param format
 */
void errExitEN(int errnum, const char *format, ...) NORETURN;

/**
 * @brief fatal Used to diagnose general errors, terminates program
 * @param format
 */
void fatal(const char *format, ...) NORETURN;

#endif // ERROR_FUNCTIONS_H
