#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <iostream>

#include "main.hpp"

class ContextTree;

class ModelUndo;

class Agent {

public:

	// construct a learning agent from the command line arguments
	Agent(options_t & options);

	// destruct the agent and the corresponding context tree
	~Agent(void);

	// current age of the agent in cycles
	age_t age(void) const;

	// the total accumulated reward across an agents lifespan
	reward_t reward(void) const;

	// the average reward received by the agent at each time step
	reward_t averageReward(void) const;

	// maximum reward in a single time instant
	reward_t maxReward(void) const;

	// minimum reward in a single time instant
	reward_t minReward(void) const;

	// number of distinct actions
	unsigned int numActions(void) const;

	// the length of the stored history for an agent
	size_t historySize(void) const;

	// length of the search horizon used by the agent
	size_t horizon(void) const;

	// generate an action uniformly at random
	action_t genRandomAction(void) const;

	// generate an action distributed according
	// to our history statistics
	action_t genAction(void) const; // TODO: implement in agent.cpp

	// generate a percept distributed according
	// to our history statistics
	percept_t genPercept(void) const; // TODO: implement in agent.cpp

	// generate a percept distributed to our history statistics, and
	// update our mixture environment model with it
	percept_t genPerceptAndUpdate(void); // TODO: implement in agent.cpp

	// update the internal agent's model of the world
	// due to receiving a percept or performing an action
	void modelUpdate(percept_t observation, percept_t reward);
	void modelUpdate(action_t action);

	// revert the agent's internal model of the world
	// to that of a previous time cycle, false on failure
	bool modelRevert(const ModelUndo &mu); // TODO: implement in agent.cpp

	// resets the agent
	void reset(void);

	// probability of selecting an action according to the
	// agent's internal model of it's own behaviour
	double getPredictedActionProb(action_t action); // TODO: implement in agent.cpp

	// get the agent's probability of receiving a particular percept
	double perceptProbability(percept_t observation, percept_t reward) const; // TODO: implement in agent.cpp

private:
	// action sanity check
	bool isActionOk(action_t action) const;

	// reward sanity check
	bool isRewardOk(reward_t reward) const;

	// encoding/decoding actions and percepts to/from symbol lists
	void encodeAction(symbol_list_t &symlist, action_t action) const;
	void encodePercept(symbol_list_t &symlist, percept_t observation, percept_t reward) const;
	action_t decodeAction(const symbol_list_t &symlist) const;
	percept_t decodeReward(const symbol_list_t &symlist) const;


	// agent properties
	unsigned int m_actions;      // number of actions
	unsigned int m_actions_bits; // number of bits to represent an action
	unsigned int m_obs_bits;     // number of bits to represent an observation
	unsigned int m_rew_bits;     // number of bits to represent a reward
	size_t m_horizon;            // length of the search horizon

	// Context Tree representing the agent's beliefs
	ContextTree *m_ct;

	// How many time cycles the agent has been alive
	age_t m_time_cycle;

	// The total reward received by the agent
	reward_t m_total_reward;

	// True if the last update was a percept update
	bool m_last_update_percept;
};


// used to store sufficient information to revert an agent
// to a copy of itself from a previous time cycle
class ModelUndo {

    public:
        // construct a save point
        ModelUndo(const Agent &agent);

        // saved state age accessor
        age_t age(void) const { return m_age; }

        // saved state reward accessor
        reward_t reward(void) const { return m_reward; }

        // saved state history size accessor
        size_t historySize(void) const { return m_history_size; }

        bool lastUpdate(void) const { return m_last_update_percept; }

    private:
        age_t m_age;
        reward_t m_reward;
        size_t m_history_size;
        bool m_last_update_percept;
};


#endif // __AGENT_HPP__
