#include <unistd.h>

#include "test.h"

int main(void) {
  struct connection conn1;
  initializeBuffers(&conn1, 5000);

  // Open a connection and check the greeting message

  connectToPort(&conn1, 10000);
  expectToRead(&conn1, "+OK Server ready (Author: *");
  expectNoMoreData(&conn1);

  // Check whether the ECHO command works

  writeString(&conn1, "ECHO Hello world!\n");
  expectToRead(&conn1, "+OK Hello world!");
  expectNoMoreData(&conn1);

  // Check whether the server rejects unknown commands

  writeString(&conn1, "BLAH\n");
  expectToRead(&conn1, "-ERR Unknown command");
  expectNoMoreData(&conn1);

  // Check whether the server properly handles partial writes

  writeString(&conn1, "ECH");
  expectNoMoreData(&conn1);
  sleep(1);
  writeString(&conn1, "O blah\nEC");
  expectToRead(&conn1, "+OK blah");
  expectNoMoreData(&conn1);
  sleep(1);
  writeString(&conn1, "HO blubb\nECHO xyz\n");
  expectToRead(&conn1, "+OK blubb");
  expectToRead(&conn1, "+OK xyz");
  expectNoMoreData(&conn1);

  // Check whether the QUIT command works

  writeString(&conn1, "QUIT\n");
  expectToRead(&conn1, "+OK Goodbye!");
  expectRemoteClose(&conn1);
  closeConnection(&conn1);

  freeBuffers(&conn1);
  return 0;
}
