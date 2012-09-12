#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "main.hpp"

// Return a number uniformly between [0, 1]
double rand01();

// Return a random integer between [0, end)
unsigned int randRange(unsigned int end);

// Return a random number between [start, end)
int randRange(int start, int end);

// Extract a value from a string
template <typename T>
void strExtract(std::string &str, T &val) {
	std::istringstream iss(str);
	iss >> val;
}

template <typename T>
T strExtract(std::string &str) {
	T val;
	strExtract(str, val);
	return val;
}

// encode/decode values to/from symbol lists
unsigned int decode(const symbol_list_t &symlist, unsigned int bits);
void encode(symbol_list_t &symlist, unsigned int value, unsigned int bits);

#endif // __UTIL_HPP__
