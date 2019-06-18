all:	sender recv

sender:	sender.o common.o
	g++ sender.o common.o -o sender

recv:	recv.o common.o
	g++ recv.o common.o -o recv

sender.o:	 sender.cpp common.h
	g++ -c sender.cpp

recv.o:	recv.cpp common.h
	g++ -c recv.cpp

common.o: common.cpp common.h
	g++ -c common.cpp

clean:
	rm -rf *.o sender recv
