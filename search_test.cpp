#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"
#include "environment.hpp"
#include "agent.hpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <cassert>

#define COIN_PROB 0.5

options_t options;

void simulate_coinflip(Agent &agent){
	int coin = rand01() < COIN_PROB;
	int guess = rand01() < 0.5 ? 1 : 0;

	agent.modelUpdate(guess);
	agent.modelUpdate(coin,coin==guess);
}

void simulate_coinflips(Agent &agent, int times){
	for(int i = 0 ; i < times; i++){
		simulate_coinflip(agent);
	}
}

int main(int argc, char *argv[]) {
	// Load configuration options
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
	simulate_coinflips(ai,2000);
	std::cout << "After 2000 flips:" << std::endl;
	std::cout << ai.prettyPrintContextTree();
	
	ModelUndo mu = ModelUndo(ai);
	for (int i = 0; i < 5; i++) {
		simulate_coinflips(ai,5);
		ai.modelRevert(mu);
		std::cout << "after revert #" << (i+1) << std::endl;
		std::cout << ai.prettyPrintContextTree();
	}
	std::cout << "after reverts" << std::endl;
	std::cout << ai.prettyPrintContextTree();

	std::cout << "Agent history: " << std::endl;
	std::cout << ai.printHistory() << std::endl;
	std::cout << "Next action: " << search(ai) << std::endl;
	
}
