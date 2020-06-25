CC = g++
CFLAGS = -g -Wall -D_GNU_SOURCE

all: tmke.o
	$(CC) $(CFLAGS) -lm -o tmke tmke.o

tmke.o: tmk.cpp ltmk.c
	$(CC) $(CFLAGS) -c tmk.cpp -o tmke.o -D_TMK1553B_LINUX -DENG

rus: tmk.o
	$(CC) $(CFLAGS) -lm -o tmk tmk.o

tmk.o: tmk.cpp ltmk.c
	$(CC) $(CFLAGS) -c tmk.cpp -D_TMK1553B_LINUX

clean:
	rm *.o
	rm tmk

realclean: clean
	rm *~


