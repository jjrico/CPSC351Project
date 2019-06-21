#include <iostream>
#include <string.h>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#define FILE_CONTENTS "Hello World"
#define KEY_FILENAME "keyfile.txt"

using std::cerr;
using std::endl;
using std::ofstream;

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


/**
 * Generates key used for both sender and receiver using ftok. If the
 * key file does not exist, it will be created.
 */
key_t generate_key() {
    struct stat fStat;
    key_t key;

  // key_t ftok(const char *path, int id);
    if (stat(KEY_FILENAME, &fStat) != 0) {
      // file does not exist, create it
      ofstream out(KEY_FILENAME);

      if (!out.good())
        goto unable;

      out << FILE_CONTENTS;

      out.close();
    }

  // generate key
  key = ftok(KEY_FILENAME, 'a');

  if (key != -1)
    return key;

unable:
  bail("could not create " KEY_FILENAME, EXIT_FAILURE);
  return 0;
}
