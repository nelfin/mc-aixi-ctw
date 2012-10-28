CPP := g++
CFLAGS := -Wall -O2

.PHONY: all
all: main;

main: *.cpp
	$(CPP) $(CFLAGS) -o $@ *.cpp
