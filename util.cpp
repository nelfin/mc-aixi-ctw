#include "util.hpp"

#include <cassert>
#include <cstdlib>


// Return a random number uniformly distributed in [0, 1]
double rand01() {
	return (double)rand() / (double)RAND_MAX;
}

// Return a random integer between [0, end)
unsigned int randRange(unsigned int end) {
	assert(end <= RAND_MAX);

	// Generate an integer between [0, end) uniformly
	int r = rand();
	const int remainder = RAND_MAX % end;
	while (r < remainder) r = rand();
	return r % end;
}

// Return a random number between [start, end)
int randRange(int start, int end) {
	assert(start < end);
	return start + randRange(end - start);
}


// Decodes the value encoded on the end of a list of symbols
unsigned int decode(const symbol_list_t &symlist, unsigned int bits) {
	assert(bits <= symlist.size());

	unsigned int value = 0;
	symbol_list_t::const_reverse_iterator it = symlist.rbegin();
	symbol_list_t::const_reverse_iterator end = it + bits;
	for( ; it != end; ++it) {
		value = (*it ? 1 : 0) + 2 * value;
	}

	return value;
}


// Encodes a value onto the end of a symbol list using "bits" symbols
void encode(symbol_list_t &symlist, unsigned int value, unsigned int bits) {
	for (unsigned int i = 0; i < bits; i++, value /= 2) {
		symbol_t sym = value & 1;
		symlist.push_back(sym);
	}
}



