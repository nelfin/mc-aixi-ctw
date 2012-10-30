#ifndef __ENVIRONMENT_HPP__
#define __ENVIRONMENT_HPP__

#include "main.hpp"

class Environment {

public:

	// Constructor: set up the initial environment percept
	// TODO: implement in inherited class

	// receives the agent's action and calculates the new environment percept
	virtual void performAction(action_t action) = 0; // TODO: implement in inherited class

	// returns true if the environment cannot interact with the agent anymore
	virtual bool isFinished(void) const { return false; } // TODO: implement in inherited class (if necessary)

	void getPercept(symbol_list_t &symlist);

	percept_t getObservation(void) const { return m_observation; }

	virtual percept_t getReward(void) const { return m_reward; }

protected: // visible to inherited classes
	action_t m_last_action;  // the last action performed by the agent
	percept_t m_observation; // the current observation
	percept_t m_reward;	  // the current reward
	int m_signed_reward; // the possibly negative version of m_reward

};


// An experiment involving flipping a biased coin and having the agent predict
// whether it will come up heads or tails. The agent receives a reward of 1
// for a correct guess and a reward of 0 for an incorrect guess.
class CoinFlip : public Environment {
public:

	// set up the initial environment percept
	CoinFlip(options_t &options);

	// receives the agent's action and calculates the new environment percept
	virtual void performAction(action_t action);

private:
	double p; // Probability of observing 1 (heads)
};

/* Tiger:

actions
	open left - 0
	open right - 1
	listen - 2

observation
	tiger behind left - 0
	tiger behind right - 1
	no observation - 2

m_gold_door
	0 - gold behind left
	1 - gold behind right
*/

class Tiger : public Environment {
public:
	Tiger(options_t &options);
	
	virtual void performAction(action_t action);
	
	percept_t getReward(void) const { return (percept_t) m_signed_reward + 100; }

private:
	double p; // Probability that the door hiding the gold is the left door.
	bool m_gold_door;
	double m_listen_chance;
	
	//Inherited:
	//action_t m_last_action;  // the last action performed by the agent
	//percept_t m_observation; // the current observation
	//percept_t m_reward;	  // the current reward
	//int m_signed_reward;
};

class GridWorld : public Environment {
public:

	// set up the initial environment percept
	GridWorld(options_t &options);

	// receives the agent's action and calculates the new environment percept
	void performAction(action_t action);

private:
	int m_x; //x dimension
	int m_y; //y dimension
};

class RPS : public Environment {
public:

	// set up the initial environment percept
	RPS(options_t &options);

	// receives the agent's action and calculates the new environment percept
	void performAction(action_t action);
	
	percept_t getReward(void) const { return m_signed_reward + 1; }

private:
	bool m_previous_rock_win; //whether the environment won the last game with rock
};

/* Kuhn Poker environment:

actions
	Pass: 0
	Bet: 1

observation (Player Card, Opponent Action)
	Player Card
		Jack: 0
		Queen: 1
		King: 2
	Opponent Action
		Pass: 0
		Bet: 1

reward: (Pot - Investment)

NOTE: 
Add an alternate environment which includes as an observation the 
outcome of the previous round (Opponent Reveal, Player Card, Opponent Action)
	Opponent Reveal
		Jack: 0
		Queen: 1
		King: 2
		Fold: 3 (no showdown)	
*/

class KuhnPoker : public Environment {
public:
	KuhnPoker(options_t &options);
	
	virtual void performAction(action_t action);
	
	percept_t getReward(void) const { return (percept_t) m_signed_reward + 2; }

private:
	unsigned int m_opponent_action;
	unsigned int m_opponent_card;
	unsigned int m_player_card;
	
	double alpha;
	double beta;
	double gamma; //The variable that defines the nash equilibrium strategy
	
	void dealCards();
	
	int getFirstNashAction();
	int getSecondNashAction();
	
	// Following are temporary variables used only within the scope of 
	// performAction
	unsigned int m_pot;
	unsigned int m_investment;
	bool m_win_flag;
	
	//Inherited:
	//action_t m_last_action;  // the last action performed by the agent
	//percept_t m_observation; // the current observation
	//percept_t m_reward;	  // the current reward
	//int m_signed_reward;
};

class Pacman : public Environment {
public:
	
	void printMap();

	// set up the initial environment percept
	Pacman(options_t &options);

	// receives the agent's action and calculates the new environment percept
	void performAction(action_t action);
	
	percept_t getReward(void) const { return (percept_t) m_signed_reward + 61; }
private:
	
	enum direction_t{UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3, NONE = 4};
	enum tile_t{EMPTY = 0, WALL = 1, PILL = 2, GHOST = 3, PACMAN = 4, FOOD = 5, FOODANDGHOST = 6, PILLANDGHOST};
	
	struct coord_t {
		int x;
		int y;
	};
	
	struct ghostinfo_t {
		coord_t pos;
		int pursue;
		int cooldown;
		bool alive;
	};
	
	void reset();
	
	percept_t setObservation();
	
	// Pacman private state variables and methods
	bool lineOfSight(coord_t curcoord, direction_t direction, tile_t seeking);
	direction_t manhattanSearch
		(coord_t curcoord, direction_t camefrom, direction_t wentto, int dist, tile_t seeking);
		coord_t makeMove(coord_t coord, direction_t move);
	void ghostMove(coord_t from, coord_t to);
	direction_t randomMove(coord_t coord);
	bool validSquare(coord_t coord);
	bool testSquare(coord_t coord, tile_t seeking);
	
	tile_t **map;
	int foodleft;
	int dimx;
	int dimy;
	int numghosts;
	
	ghostinfo_t *ghosts;	
	coord_t pacman;
	bool power;
	int powertime;
};

class Composite : public Environment {
public:
	// 
	Composite(options_t &options);
	
	virtual void performAction(action_t action);
	
	percept_t getReward(void) const { return (percept_t) m_signed_reward + 0; }
private:
	// Private variables go here
	int m_environment[10];
	int m_start[10];
	int m_current_cycle;
	int m_current_environment;
	int m_last_environment;

	// For extracting the environment sequence and changeover times 
	std::string environmentCode(int i);
	std::string startCode(int i);


	// For keeping track of the options
	options_t m_options;

	// Environment variables

	// Coin Flip
	double p; // For storing the chance of heads

	void initialise(options_t &options, int environment);
	void resolveAction(action_t action, int environment);
};

#endif // __ENVIRONMENT_HPP__
