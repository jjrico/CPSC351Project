#include <iostream>
#include <string.h>

using std::cerr;
using std::endl;

void cleanUp();

/**
 * Prints a failure message and cleans up any allocated resources
 * @param msg - failure message to display
 * @param exitCode - program exit code
 */
void bail(const char* msg, int exitCode) {
  if (errno != 0)
    perror(msg);
  else cerr << msg << endl;

  cleanUp();
  exit(exitCode);
}
