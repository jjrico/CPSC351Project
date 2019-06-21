#include "msg.h" /* For the message struct */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the allocated message queue
 */
void init(int &shmid, int &msqid, void *&sharedMemPtr) {
  /*
  1. Create a file called keyfile.txt containing string "Hello world" (you may
  do so manually or from the code).
  2. Use ftok("keyfile.txt", 'a') in order to generate the key. */
  std::cout << "\tCreating unique key..." << std::endl;
  key_t key = generate_key();

  /*
  3. Use will use this key in the TODO's below. Use the same key for the queue
     and the shared memory segment. This also serves to illustrate the
  difference between the key and the id used in message queues and shared
  memory. The key is like the file name and the id is like the file object.
  Every System V object on the system has a unique id, but different objects may
  have the same key.
  */

  /* Get the id of the shared memory segment. The size of the segment must
   * be SHARED_MEMORY_CHUNK_SIZE */
   std::cout << "\tObtaining id to shared memory..." << std::endl;
  if (-1 == (shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, S_IRUSR))) {
      bail("could not acquire shared memory id", errno);
  }

  /* Attach to the shared memory */
  std::cout << "\tAttaching to shared memory..." << std::endl;
  if ((void*)-1 == (sharedMemPtr = (void *)shmat(shmid, NULL, 0))) {
    bail("could not attach to shared memory segment", errno);
  }

  /* Attach to the message queue */
    std::cout << "\tAttaching to message queue..." << std::endl;
  if (-1 == (msqid = msgget(key, S_IRUSR | S_IWUSR))) {
    bail("could not attach to message queue", errno);
  }

  /* Store the IDs and the pointer to the shared memory region in the
   * corresponding function parameters */
}

/**
 * Performs the cleanup functions
 */
void cleanUp() {
  /* Detach from shared memory */
  std::cout << "\tDetaching from shared memory..." << std::endl;
  shmdt(sharedMemPtr);
}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char *fileName) {

  /* A buffer to store message we will send to the receiver. */
  message sndMsg;

  /* A buffer to store message received from the receiver. */
  ackMessage rcvMsg;

  /* The number of bytes sent */
  unsigned long numBytesSent = 0;

  /* Open the file */
  FILE *fp = fopen(fileName, "r");

  /* Was the file open? */
  if (!fp) {
    perror("fopen");
    exit(-1);
  }

  /* Read the whole file */
  while (!feof(fp)) {
    /* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and
     * store them in shared memory.  fread() will return how many bytes it has
     * actually read. This is important; the last chunk read may be less than
     * SHARED_MEMORY_CHUNK_SIZE.
     */

    sndMsg.size =
        fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp);

    if (sndMsg.size < 0) {
      fclose(fp);
      bail("failed to read file data", errno);
    }

    /* count the number of bytes sent. */

		numBytesSent += sndMsg.size;

    /* Send a message to the receiver telling him that the
     * data is ready to be read (message of type SENDER_DATA_TYPE).
     */
		sndMsg.mtype = SENDER_DATA_TYPE;
		if (-1 == msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0)) {
      fclose(fp);
      bail("failed to send next data message", errno);
    }

    /* Wait until the receiver sends us a message of type
     * RECV_DONE_TYPE telling us that he finished saving a chunk of
     * memory.
     */
		 if (-1 == msgrcv(msqid, &rcvMsg, sizeof(ackMessage) - sizeof(long), RECV_DONE_TYPE, 0)) {
       fclose(fp);
       bail("failed to receive ack message", errno);
     }
  }

  /** once we are out of the above loop, we have finished sending the
   * file. Lets tell the receiver that we have nothing more to send. We will do
   * this by sending a message of type SENDER_DATA_TYPE with size field set to
   * 0.
   */
	 sndMsg.mtype = SENDER_DATA_TYPE;
	 sndMsg.size = 0;
	 if (-1 == msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0))
    perror("msgsnd");

  /* Close the file */
  fclose(fp);

  return numBytesSent;
}

/**
 * Used to send the name of the file to the receiver
 * @param fileName - the name of the file to send
 */
void sendFileName(const char *fileName) {
  /* Get the length of the file name */
  int fileNameSize = strlen(fileName);

  /* Make sure the file name does not exceed
   * the maximum buffer size in the fileNameMsg
   * struct. If exceeds, then terminate with an error.
   */
	 if(fileNameSize > MAX_FILE_NAME_SIZE) {
		 bail("Max File Name Size Exceeded", EXIT_FAILURE);
	 }

  /* Create an instance of the struct representing the message
   * containing the name of the file.
   */
	 fileNameMsg fMsg;

  /* Set the message type FILE_NAME_TRANSFER_TYPE */
	fMsg.mtype = FILE_NAME_TRANSFER_TYPE;
  /* Set the file name in the message */
  strncpy(fMsg.fileName, fileName, MAX_FILE_NAME_SIZE);

  /* Send the message using msgsnd */
	if (-1 == msgsnd(msqid, &fMsg, sizeof(fileNameMsg) - sizeof(long), 0)) {
    bail("could not send file name message", errno);
  }
}

int main(int argc, char **argv) {

  /* Check the command line arguments */
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
    exit(-1);
  }

  // make sure file actually exists
  struct stat fStat;

  if (stat(argv[1], &fStat) != 0) {
    std::cerr << "File not found: " << argv[1] << std::endl;
    exit(EXIT_FAILURE);
  }

  /* Connect to shared memory and the message queue */
  std::cout << "Connecting to shared memory..." << std::endl;
  init(shmid, msqid, sharedMemPtr);

  /* Send the name of the file */
  std::cout << "Sending file name: " << argv[1] << std::endl;
  sendFileName(argv[1]);

  /* Send the file */
  std::cout << "Sending file contents..." << std::endl;
  fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));

  /* Cleanup */
  std::cout << "Cleaning up..." << std::endl;
  cleanUp();

  return 0;
}
