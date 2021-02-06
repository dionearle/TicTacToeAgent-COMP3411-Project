#  Makefile
#  Nine-Board Tic-Tac-Toe
#  COMP3411/9414/9814 Artificial Intelligence
#  Dion Earle, Assignment 3

CC = gcc
CFLAGS = -Wall -g -O3

default: agent

agent: agent.o client.o game.o common.h agent.h game.h
	$(CC) $(CFLAGS) -o agent agent.o client.o game.o

servt: servt.o game.o common.h game.h agent.h
	$(CC) $(CFLAGS) -o servt servt.o game.o

all: servt agent

%o:%c common.h agent.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f servt agent *.o
