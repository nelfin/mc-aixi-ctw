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
	//Runs a coinflip example if the agent guesses randomly and the coin is random
	for(int i = 0; i < 3000; i++) {
	    int guess = rand01() < 0.5 ? 1 : 0;
	    int coin = rand01() < 0.5 ? 1 : 0;
		ctw->update(guess);
		ctw->update(coin);
		ctw->update(coin==guess ? 1 : 0);
	}
	
	//Action and observation are 0, 1 so ctw_tree would predict reward 0 if it "understood" the game
	ctw->update(0);
	ctw->update(1);
	std::cout << ctw->prettyPrint();
	std::cout << "Sequence: " << ctw->printHistory() << std::endl;
	std::cout << "Next: "<< ctw->predictNext() << std::endl;
}
