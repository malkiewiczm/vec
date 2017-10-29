CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -Wcast-qual	\
-Wstrict-prototypes -Wmissing-prototypes -Werror -O2
LDLIBS = -lm
TARGET = ./bin/vec
OBJECTS = ./src/main.o ./src/stack.o

.PHONY: all clean install

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(OBJECTS)

install: all
	cp $(TARGET) /usr/bin/vec
