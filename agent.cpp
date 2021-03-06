#include "agent.hpp"

#include <cassert>

#include "predict.hpp"
#include "search.hpp"
#include "util.hpp"


// construct a learning agent from the command line arguments
Agent::Agent(options_t & options) {
	std::string s;

	strExtract(options["agent-actions"], m_actions);
	strExtract(options["agent-horizon"], m_horizon);
	strExtract(options["mc-simulations"], m_simulations);
	strExtract(options["observation-bits"], m_obs_bits);
	strExtract<unsigned int>(options["reward-bits"], m_rew_bits);

	// calculate the number of bits needed to represent the action
	for (unsigned int i = 1, c = 1; i < m_actions; i *= 2, c++) {
		m_actions_bits = c;
	}

	m_ct = new ContextTree(strExtract<unsigned int>(options["ct-depth"]));

	reset();
}

Agent::Agent(const Agent &a) {
	m_actions = a.m_actions;
	m_horizon = a.m_horizon;
	m_simulations = a.m_simulations;
	m_obs_bits = a.m_obs_bits;
	m_rew_bits = a.m_rew_bits;
	m_actions_bits = a.m_actions_bits;
	m_ct = new ContextTree(*a.m_ct);
}


// destruct the agent and the corresponding context tree
Agent::~Agent(void) {
	if (m_ct) delete m_ct;
}

// print out the agent's history
std::string Agent::printHistory(void) const {
	return m_ct->printHistory();
}

// print out the agent's context tree
std::string Agent::prettyPrintContextTree() const {
	return m_ct->prettyPrint();
}

int Agent::numSimulations(void) const {
	return m_simulations;
}


// current age of the agent in cycles
age_t Agent::age(void) const {
	return m_time_cycle;
}

// the total accumulated reward across an agents lifespan
reward_t Agent::reward(void) const {
	return m_total_reward;
}


// the average reward received by the agent at each time step
reward_t Agent::averageReward(void) const {
	return age() > 0 ? reward() / reward_t(age()) : 0.0;
}

// maximum reward in a single time instant
reward_t Agent::maxReward(void) const {
	return reward_t((1 << m_rew_bits) - 1);
}


// minimum reward in a single time instant
reward_t Agent::minReward(void) const {
	return 0.0;
}


// number of distinct actions
unsigned int Agent::numActions(void) const {
	return m_actions;
}


// the length of the stored history for an agent
size_t Agent::historySize(void) const {
	return m_ct->historySize();
}


// length of the search horizon used by the agent
size_t Agent::horizon(void) const {
	return m_horizon;
}



// generate an action uniformly at random
action_t Agent::genRandomAction(void) const {
	return randRange(m_actions);
}

// generate an action distributed according
// to our history statistics
action_t Agent::genAction(void) const {
	symbol_list_t action_syms;
	m_ct->genRandomSymbols(action_syms, m_actions_bits);
	return decodeAction(action_syms);
}


// generate a percept distributed according
// to our history statistics
void Agent::genPercept(percept_t *observation, percept_t *reward) {
	symbol_list_t percept;
	size_t totalBits = m_rew_bits + m_obs_bits;
	
	m_ct->genRandomSymbols(percept, totalBits);
	*reward = decodeReward(percept);
	for (unsigned int i = 0; i < m_rew_bits; i++) {
		percept.pop_back();
	}
	*observation = decodeObservation(percept);
   
}


// generate a percept distributed to our history statistics, and
// update our mixture environment model with it
void Agent::genPerceptAndUpdate(percept_t *observation, percept_t *reward) {
	symbol_list_t percept;
	size_t totalBits = m_rew_bits + m_obs_bits;

	m_ct->genRandomSymbolsAndUpdate(percept, totalBits);
	*reward = decodeReward(percept);
	for (unsigned int i = 0; i < m_rew_bits; i++) {
	percept.pop_back();
	}
	*observation = decodeObservation(percept);

	// Update agent properties
	m_total_reward += *reward;
	m_last_update_percept = true;
}


// Update the agent's internal model of the world after receiving a percept
void Agent::modelUpdate(percept_t observation, percept_t reward) {
	// Update internal model
	symbol_list_t percept;
	encodePercept(percept, observation, reward);

	m_ct->update(percept);
	// m_history is updated by ContextTree::update


	// Update other properties
	m_total_reward += reward;
	m_last_update_percept = true;
}


// Update the agent's internal model of the world after performing an action
void Agent::modelUpdate(action_t action) {
	assert(isActionOk(action));
	assert(m_last_update_percept == true);

	// Update internal model
	symbol_list_t action_syms;
	encodeAction(action_syms, action);
	m_ct->updateHistory(action_syms);
	// m_history is updated by ContextTree::update

	m_time_cycle++;
	m_last_update_percept = false;
}


// revert the agent's internal model of the world
// to that of a previous time cycle, false on failure
bool Agent::modelRevert(const ModelUndo &mu) {
	
	// correct asserts
	try {
	assert(mu.age() <= age());
	assert(mu.historySize() <= historySize());
	}
	catch (char *str) {
	return false;
	}

	// remove all new items from the historySize
	// revert the CTW at each step
	for (size_t i = historySize(); i > mu.historySize(); ) {
		if (m_last_update_percept) {
			// revert an observation and reward
			for (size_t j = 0; j < (m_obs_bits + m_rew_bits); j++) {
				m_ct->revert();
			}
			i -= (m_obs_bits + m_rew_bits);
		} else {
			// revert an action
			m_ct->revertHistory(i - m_actions_bits);
			i -= m_actions_bits;
		}
		m_last_update_percept = !m_last_update_percept;
	}
	// revert other states
	m_time_cycle = mu.age();
	m_total_reward = mu.reward();
	m_last_update_percept = mu.lastUpdate();
	return true;
}


void Agent::reset(void) {
	m_ct->clear();

	m_time_cycle = 0;
	m_total_reward = 0.0;
}

// probability of selecting an action according to the
// agent's internal model of it's own behaviour
double Agent::getPredictedActionProb(action_t action) {
	return NULL; // TODO: implement
}


// get the agent's probability of receiving a particular percept
double Agent::perceptProbability(percept_t observation, percept_t reward) const {
	return NULL; // TODO: implement
}


// action sanity check
bool Agent::isActionOk(action_t action) const {
	return action < m_actions;
}


// reward sanity check
bool Agent::isRewardOk(reward_t reward) const {
	return reward >= minReward() && reward <= maxReward();
}


// Encodes an action as a list of symbols
void Agent::encodeAction(symbol_list_t &symlist, action_t action) const {
	symlist.clear();

	encode(symlist, action, m_actions_bits);
}

// Encodes a percept (observation, reward) as a list of symbols
void Agent::encodePercept(symbol_list_t &symlist, percept_t observation, percept_t reward) const {
	symlist.clear();

	encode(symlist, observation, m_obs_bits);
	encode(symlist, reward, m_rew_bits);
}

// New function to decode observation from a list of symbols
percept_t Agent::decodeObservation(const symbol_list_t &symlist) const {
  return decode(symlist, m_obs_bits);
}

// Decodes the s/observation/action from a list of symbols
action_t Agent::decodeAction(const symbol_list_t &symlist) const {
	return decode(symlist, m_actions_bits);
}


// Decodes the reward from a list of symbols
percept_t Agent::decodeReward(const symbol_list_t &symlist) const {
	return decode(symlist, m_rew_bits);
}

bool Agent::getLastUpdate(void) const {
	return m_last_update_percept;
}

ModelUndo::~ModelUndo(void) {
	delete m_revert_clone;
}

// used to revert an agent to a previous state
// defunct deep copy code used for testing
ModelUndo::ModelUndo(const Agent &agent) {
	m_revert_clone = new Agent(agent);
	m_age		  = agent.age();
	m_reward	   = agent.reward();
	m_history_size = agent.historySize();
	m_last_update_percept = agent.getLastUpdate();
}
