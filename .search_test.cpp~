#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"
#include "environment.hpp"
#include "agent.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>

int main(int argc, char *argv[]) {
	// Load configuration options
	options_t options;
	
	// Default configuration values
	options["ct-depth"] = "4";
	options["agent-horizon"] = "16";
	options["exploration"] = "0";	 // do not explore
	options["explore-decay"] = "1.0"; // exploration rate does not decay
	options["mc-simulations"] = "32";
	options["agent-actions"] = "2";
	options["observation-bits"] = "1";
	options["reward-bits"] = "1";
	
	Agent ai(options);
	for(int i = 0; i < 2000; i++){
		int coin = rand01() < 0;
		int guess = rand01() < 0.5 ? 1 : 0;
		
		ai.modelUpdate(guess);
		ai.modelUpdate(coin,coin==guess);
	}
	std::cout << "before:" << std:endl;
	std::cout << ai.prettyPrintContextTree();
	ModelUndo mu = ModelUndo(ai);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 200; j++) {
			int coin = rand01() < 0;
			int guess = rand01() < 0.5 ? 1 : 0;
		
			ai.modelUpdate(guess);
			ai.modelUpdate(coin,coin==guess);
		}
		ai.modelRevert(mu);
		std::cout << "after revert" << std::endl;
		std::cout << ai.prettyPrintContextTree();
	}
	std::cout << "after revert" << std::endl;
	std::cout << ai.prettyPrintContextTree();

	std::cout << "Agent history: " << std::endl;
	std::cout << ai.printHistory() << std::endl;
	std::cout << "Next action: " << search(ai) << std::endl;
	
}
