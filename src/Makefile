# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -pthread

all: mirr
writefileexe: writefile

writefile: writefile.o global.o
	$(CC) $(CFLAGS) writefile.o global.o -lm -o writefile

mirr: mirr.o global.o tree.o backup.o
	$(CC) $(CFLAGS) mirr.o global.o tree.o backup.o -lm -o mirr


mirr.o: mirr.c
	$(CC) $(CFLAGS) -c mirr.c

backup.o: backup.c
	$(CC) $(CFLAGS) -c backup.c

global.o: global.c
	$(CC) $(CFLAGS) -c global.c

tree.o: tree.c
	$(CC) $(CFLAGS) -c tree.c

writefile.o: writefile.c
	$(CC) $(CFLAGS) -c writefile.c

clean:
	rm -fv *.o
	rm mirr
	rm writefile
	rm -f ../tmp/*

cunit:
	sudo apt-get install libcunit1 libcunit1-doc libcunit1-dev
