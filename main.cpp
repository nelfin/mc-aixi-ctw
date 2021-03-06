#include "main.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>


#include "agent.hpp"
#include "environment.hpp"
#include "search.hpp"
#include "util.hpp"

#define DEBUGMODE false

// Streams for logging
std::ofstream logFile;		// A verbose human-readable log
std::ofstream compactLog;	// A compact comma-separated value log

// The main agent/environment interaction loop
void mainLoop(Agent &ai, Environment &env, options_t &options) {
	// Determine exploration options
	bool explore = options.count("exploration") > 0;
	double explore_rate, explore_decay;
	if (explore) {
		strExtract(options["exploration"], explore_rate);
		strExtract(options["explore-decay"], explore_decay);
		assert(0.0 <= explore_rate && explore_rate <= 1.0);
		assert(0.0 <= explore_decay && explore_decay <= 1.0);
	}


	// Determine termination age
	bool terminate_check = options.count("terminate-age") > 0;
	age_t terminate_age;
	if (terminate_check) {
		strExtract(options["terminate-age"], terminate_age);
		assert(0 <= terminate_age);
	}

	// Agent/environment interaction loop
	for (unsigned int cycle = 1; !env.isFinished(); cycle++) {

		// check for agent termination
		if (terminate_check && ai.age() > terminate_age) {
			logFile << "info: terminating agent" << std::endl;
			break;
		}

		// Get a percept from the environment
		percept_t observation = env.getObservation();
		percept_t reward = env.getReward();

		// Update agent's environment model with the new percept
		ai.modelUpdate(observation, reward);

		// Determine best exploitive action, or explore
		action_t action;
		bool explored = false;
		
		if (DEBUGMODE){
		 	//SPECIFY ACTIONS ON COMMAND LINE FOR TESTING
		 	char userinput[50];
		 	
			assert(scanf("%s", userinput)==1);
		 	if (userinput[0] == 'w'){
		 		action = 0;
		 	} else if (userinput[0] == 'd'){
		 		action = 1;	
		 	} else if (userinput[0] == 's'){
		 		action = 2;	
		 	} else if (userinput[0] == 'a'){
		 		action = 3;	
		 	} else action = atoi(userinput);
		} else {		
			// proper code structure
			if (explore && rand01() < explore_rate) {
				explored = true;
			
				action = ai.genRandomAction();	
			}
			else {
				action = search(ai);
			}
		}

		// Send an action to the environment
		env.performAction(action);

		// Update agent's environment model with the chosen action
		ai.modelUpdate(action);

		// LogFile this turn
		logFile << "cycle: " << cycle << std::endl;
		logFile << "observation: " << observation << std::endl;
		logFile << "reward: " << reward << std::endl;
		logFile << "action: " << action << std::endl;
		logFile << "explored: " << (explored ? "yes" : "no") << std::endl;
		logFile << "explore rate: " << explore_rate << std::endl;
		logFile << "total reward: " << ai.reward() << std::endl;
		logFile << "average reward: " << ai.averageReward() << std::endl;

		// LogFile the data in a more compact form
		compactLog << cycle << ", " << observation << ", " << reward << ", "
				<< action << ", " << explored << ", " << explore_rate << ", "
				<< ai.reward() << ", " << ai.averageReward() << std::endl;

		// Print to standard output when cycle == 2^n
		if ((cycle & (cycle - 1)) == 0) {
			std::cout << "cycle: " << cycle << std::endl;
			std::cout << "average reward: " << ai.averageReward() << std::endl;
			//std::cout << "agent CTW tree: " << std::endl << ai.prettyPrintContextTree() << std::endl;
			//std::cout << "agent history: " << ai.printHistory() << std::endl;
			if (explore) {
				std::cout << "explore rate: " << explore_rate << std::endl;
			}
		} else {
		  //std::cout << std::endl << std::endl << std::endl; // WHY DO IT ;___;
		}

		// Update exploration rate
		if (explore) explore_rate *= explore_decay;

	}

	// Print summary to standard output
	std::cout << std::endl << std::endl << "SUMMARY" << std::endl;
	std::cout << "agent age: " << ai.age() << std::endl;
	std::cout << "average reward: " << ai.averageReward() << std::endl;
}


// Populate the 'options' map based on 'key=value' pairs from an input stream
void processOptions(std::ifstream &in, options_t &options) {
	std::string line;
	size_t pos;

	for (int lineno = 1; in.good(); lineno++) {
		std::getline(in, line);

		// Ignore # comments
		if ((pos = line.find('#')) != std::string::npos) {
			line = line.substr(0, pos);
		}

		// Remove whitespace
		while ((pos = line.find(" ")) != std::string::npos)
			line.erase(line.begin() + pos);
		while ((pos = line.find("\t")) != std::string::npos)
			line.erase(line.begin() + pos);


		// Split into key/value pair at the first '='
		pos = line.find('=');
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		// Check that we have parsed a valid key/value pair. Warn on failure or
		// set the appropriate option on success.
		if (pos == std::string::npos) {
			std::cerr << "WARNING: processOptions skipping line " << lineno << " (no '=')" << std::endl;
		}
		else if (key.size() == 0) {
			std::cerr << "WARNING: processOptions skipping line " << lineno << " (no key)" << std::endl;
		}
		else if (value.size() == 0) {
			std::cerr << "WARNING: processOptions skipping line " << lineno << " (no value)" << std::endl;
		}
		else {
			options[key] = value; // Success!
			std::cout << "OPTION: '" << key << "' = '" << value << "'" << std::endl;
		}

	}
}

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 3) {
		std::cerr << "ERROR: Incorrect number of arguments" << std::endl;
		std::cerr << "The first argument should indicate the location of the configuration file and the second (optional) argument should indicate the file to logFile to." << std::endl;
		return -1;
	}

	// Set up logging
	std::string log_file = argc < 3 ? "log" : argv[2];
	logFile.open((log_file).c_str());
	compactLog.open((log_file + ".csv").c_str());

	// Print header to compactLog
	compactLog << "cycle, observation, reward, action, explored, explore_rate, total reward, average reward" << std::endl;


	// Load configuration options
	options_t options;

	// Default configuration values
	options["ct-depth"] = "4";
	options["agent-horizon"] = "16";
	options["exploration"] = "0";	 // do not explore
	options["explore-decay"] = "1.0"; // exploration rate does not decay
	options["mc-simulations"] = "100";

	// Read configuration options
	std::ifstream conf(argv[1]);
	if (!conf.is_open()) {
		std::cerr << "ERROR: Could not open file '" << argv[1] << "' now exiting" << std::endl;
		return -1;
	}
	processOptions(conf, options);
	conf.close();

	// Set up the environment
	Environment *env = NULL;

	// TODO: instantiate the environment based on the "environment-name"
	// option. For any environment you do not implement you may delete the
	// corresponding if statement.
	// NOTE: you may modify the options map in order to set quantities such as
	// the reward-bits for each particular environment. See the coin-flip
	// experiment for an example.
	std::string environment_name = options["environment"];
	if (environment_name == "coin-flip") {
		env = new CoinFlip(options);
		options["ct-depth"] = "4";
		options["agent-actions"] = "2";
		options["observation-bits"] = "1";
		options["reward-bits"] = "1";
	}
	else if (environment_name == "1d-maze") {
		// TODO: instantiate "env" (if appropriate)
	}
	else if (environment_name == "cheese-maze") {
		// TODO: instantiate "env" (if appropriate)
	}
	else if (environment_name == "tiger") {
		env = new Tiger(options);
		options["ct-depth"] = "36";
		options["agent-horizon"] = "5";
		options["agent-actions"] = "3";
		options["observation-bits"] = "2";
		options["reward-bits"] = "7";
	}
	else if (environment_name == "extended-tiger") {
		// TODO: instantiate "env" (if appropriate)
	}
	else if (environment_name == "4x4-grid") {
		env = new GridWorld(options);
		options["ct-depth"] = "36";
		options["agent-horizon"] = "12";
		options["agent-actions"] = "4";
		options["observation-bits"] = "1";
		options["reward-bits"] = "1";
	}
	else if (environment_name == "tictactoe") {
		// TODO: instantiate "env" (if appropriate)
	}
	else if (environment_name == "biased-rock-paper-scissor") {
		env = new RPS(options);
		options["ct-depth"] = "32";
		options["agent-horizon"] = "4";
		options["agent-actions"] = "3";
		options["observation-bits"] = "1";
		options["reward-bits"] = "1";
	}
	else if (environment_name == "kuhn-poker") {
		env = new KuhnPoker(options);
		options["ct-depth"] = "42";
		options["agent-horizon"] = "2";
		options["agent-actions"] = "2";
		options["observation-bits"] = "3";
		options["reward-bits"] = "2";
	}
	else if (environment_name == "pacman") {
		env = new Pacman(options);
		options["ct-depth"] = "96";
		options["agent-horizon"] = "4";
		options["agent-actions"] = "4";
		options["observation-bits"] = "16";
		options["reward-bits"] = "8";
	}
	else if (environment_name == "composite") {
		env = new Composite(options);
		// THE FOLLOWING VALUES ARE FOR TESTING
		options["mc-simulations"] = "30";
		if (options["environment2"] == "2"){ // With Tiger
			options["ct-depth"] = "30";
			options["agent-horizon"] = "5";
			options["agent-actions"] = "3";
			options["observation-bits"] = "2";
			options["reward-bits"] = "7";
		} else { // Just coinflip
			options["ct-depth"] = "4";
			options["agent-actions"] = "2";
			options["observation-bits"] = "1";
			options["reward-bits"] = "1";
		}
	}
	else {
		std::cerr << "ERROR: unknown environment '" << environment_name << "'" << std::endl;
		return -1;
	}

	// Set up the agent
	Agent ai(options);

	// Run the main agent/environment interaction loop
	mainLoop(ai, *env, options);

	logFile.close();
	compactLog.close();

	return 0;
}
