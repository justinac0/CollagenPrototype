CC=clang++
CFLAGS=-pedantic -W -Wextra -std=c++11
LDFLAGS=-lraylib

SOURCES=$(wildcard src/*.cpp src/**/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
APP_NAME=collagen_prototype
EXECUTABLE=bin/$(APP_NAME)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(EXECUTABLE)

clean:
	rm $(EXECUTABLE) $(OBJECTS)

run:
	./$(EXECUTABLE)

.PHONY: all clean run

