# Lucifer V1.0
# By Peter Todd (c) 1999
#
# Note that the dos edit command doesn't work. When your press tab it
# inserts spaces instead of a tab. :(

CC = gcc
CFLAGS = -g -Wall -D_GNU_SOURCE

all: randbcrt.o
	$(CC) $(CFLAGS) -lm -o randbcrt randbcrt.o

randbcrt.o: randbcrt.c ltmk.c randbcrt.cpp
	$(CC) $(CFLAGS) -c randbcrt.c

clean:
	rm *.o
	rm randbcrt

realclean: clean
	rm *~


