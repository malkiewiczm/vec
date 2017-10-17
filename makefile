CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow -Wcast-qual	\
-Wstrict-prototypes -Wmissing-prototypes -Werror -O2
LDLIBS = -lm
TARGET = ./bin/vec
SOURCES = $(wildcard ./src/*.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(OBJECTS)
