#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <fstream>
#include <map>
#include <string>
#include <vector>

// Streams for logging (defined in main.cpp)
extern std::ofstream log;
extern std::ofstream compactLog;

// symbols that can be predicted
typedef bool symbol_t;

// a list of symbols
typedef std::vector<symbol_t> symbol_list_t;

// describe the reward accumulated by an agent
typedef double reward_t;

// describe a percept (observation or reward)
typedef unsigned int percept_t;

// describe the age of an agent
typedef unsigned long long age_t;

// describes an agent action
typedef unsigned int action_t;

// the program's keyword/value option pairs
typedef std::map<std::string, std::string> options_t;

#endif // __MAIN_HPP__
