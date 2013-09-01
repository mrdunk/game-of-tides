CXXFLAGS = -std=c++0x -g -Wall
CC = g++
CFLAGS+= -g -Wall
LDFLAGS=
LIBS=-lGL -lGLU -lglut -lGLEW -pthread

game: signaling.o main.o viewport.o data.o map.o vessel.o cockpit.o pathfind.o console.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm *.o game

