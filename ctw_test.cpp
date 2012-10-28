#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>

int main(int argc, char *argv[]) {
	size_t ct_size = 4;
	ContextTree *ctw = new ContextTree(ct_size);
	for(int i = 0; i < 20; i++) {
		ctw->update(1); //This is the symbol it will try to guess
		ctw->update(1);
		ctw->update(0);
		ctw->update(0);
	}
	//ctw->update(0);
	//ctw->revert();
	std::cout << ctw->prettyPrint();
	std::cout << "Sequence: " << ctw->printHistory();
	std::cout << "Next: "<< ctw->predictNext() << std::endl;
}