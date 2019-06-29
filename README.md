# OS-Sender-Reciever-LINUX
# Jeremy Rico

This process uses Linux system calls to create two programs which create a shared memory segment to transfer data. The two
processes run cocurrently until all data has been transferred from the sender to reciever, at which point the sender sends a
message of size 0 which terminates the process. Once the recieved has finished obtaining data from the sender, it creates a 
new file with the name <filename>__recv.

Included files:

- Makefile - compiles all programs
- Design of Sender and Receiver.pdf - Describes innerworkings of all programs along with a design flowchart that illustrates
                                     the workflow of the processes.
- common.h & common.cpp - header file containing common functions used by both the sender and reciever program
- keyfile.txt - used to create a unique key so the sender and reciever can access the same shared memory region
- msg.h - contains structs for different types of messages used by the sender and reciever programs
- recv.cpp - reciever program
- sender.cpp - sender program
- Test1.txt, Test2.txt, Test3.txt - test files

