CPP := g++
CFLAGS := -Wall -O2 -g

.PHONY: all
all: main ctw_test; 

main: agent.cpp environment.cpp main.cpp predict.cpp search.cpp util.cpp
	$(CPP) $(CFLAGS) -o $@ agent.cpp environment.cpp main.cpp predict.cpp search.cpp util.cpp

ctw_test: agent.cpp ctw_test.cpp predict.cpp search.cpp util.cpp
	$(CPP) $(CFLAGS) -o $@ agent.cpp ctw_test.cpp predict.cpp search.cpp util.cpp