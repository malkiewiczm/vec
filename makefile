MAKEFLAGS += Rr
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -Wshadow -O2
TARGET = vec

.PHONY: all clean install

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)

install: all
	cp $(TARGET) /usr/bin/vec
