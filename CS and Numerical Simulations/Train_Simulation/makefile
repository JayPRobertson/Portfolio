CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

OBJ1 = mts.o
OBJ2 = train.o

all: mts

mts: $(OBJ1) $(OBJ2)
	$(CC) $(CFLAGS) -o mts $(OBJ1) $(OBJ2)

mts.o: mts.c train.o
	$(CC) $(CFLAGS) -c mts.c

train.o: train.c train.h
	$(CC) $(CFLAGS) -c train.c

clean:
	rm -rf *.o mts