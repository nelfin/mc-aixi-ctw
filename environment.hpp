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
	percept_t m_reward;      // the current reward

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
	int m_signed_reward; // the possibly negative version of m_reward
};

#endif // __ENVIRONMENT_HPP__
