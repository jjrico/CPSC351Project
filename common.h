#ifndef COMMON_H
#define COMMON_H
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>

void bail(const char* msg, int exitCode);
key_t generate_key();

#endif // COMMON_H
