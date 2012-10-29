#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>

int main(int argc, char *argv[]) {
	size_t ct_size = 4;
	ContextTree ctw(ct_size);
	//Runs a coinflip example if the agent guesses randomly and the coin is random
	for(int i = 0; i < 1000; i++) {
	    int guess = rand01() < 0.5 ? 1 : 0;
	    int coin = rand01() < 0.5 ? 1 : 0;
		ctw.update(guess);
		ctw.update(coin);
		ctw.update(coin==guess ? 1 : 0);
	}
	
	//Action and observation are 0, 1 so ctw_tree would predict reward 0 if it "understood" the game
// 	std::cout << ctw->prettyPrint();
// 	std::cout << "Sequence: " << ctw->printHistory() << std::endl;
// 	std::cout << "Next: "<< ctw->predictNext() << std::endl;
	std::cout << "Original:" << std::endl << ctw.prettyPrint();
	ContextTree copy_tree(ctw);
	std::cout << "Copy:" << std::endl << copy_tree.prettyPrint();

	ctw.update(0);
	copy_tree.update(1);
	std::cout << "Original + 0:" << std::endl << ctw.prettyPrint();
	//std::cout << "Sequence: " << ctw.printHistory() << std::endl;
	std::cout << "Copy + 1:" << std::endl << copy_tree.prettyPrint();
	//std::cout << "Sequence: " << copy_tree.printHistory() << std::endl;
}
