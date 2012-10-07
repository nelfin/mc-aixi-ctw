#include "search.hpp"

#include "agent.hpp"

typedef unsigned long long visits_t;


// search options
static const visits_t     MinVisitsBeforeExpansion = 1;
static const unsigned int MaxDistanceFromRoot  = 100;
//static size_t             MaxSearchNodes;

// contains information about a single "state"
class SearchNode {

public:

	SearchNode(bool is_chance_node);

	// determine the next action to play
	action_t selectAction(Agent &agent) const; // TODO: implement

	// determine the expected reward from this node
	reward_t expectation(void) const { return m_mean; }

	// perform a sample run through this node and it's children,
	// returning the accumulated reward from this sample run
	reward_t sample(Agent &agent, unsigned int dfr); // TODO: implement

	// number of times the search node has been visited
	visits_t visits(void) const { return m_visits; }

private:

	bool m_chance_node; // true if this node is a chance node, false otherwise
	double m_mean;      // the expected reward of this node
	visits_t m_visits;  // number of times the search node has been visited

	// TODO: decide how to reference child nodes
	//  e.g. a fixed-size array
};

// simulate a path through a hypothetical future for the agent within it's
// internal model of the world, returning the accumulated reward.
static reward_t playout(Agent &agent, unsigned int playout_len) {
	return 0; // TODO: implement
}

// determine the best action by searching ahead using MCTS
extern action_t search(Agent &agent) {
	return agent.genRandomAction(); // TODO: implement

}

// determine the next action to play
action_t SearchNode::selectAction(Agent &agent) const {
    // req: a search tree \Psi
    // req: a history h
    // req: an exploration/exploitation constant C
}

// perform a sample run through this node and it's children,
// returning the accumulated reward from this sample run
reward_t SearchNode::sample(Agent &agent, unsigned int dfr) {
    // req: a search tree \Psi
    // req: a history h
    // req: a remaining search horizon m
    // XXX: is dfr = m?
    reward_t reward;
    if (dfr == 0) {
        return reward_t(0.0);
    } else if (m_chance_node) {
        // chance node business
    } else if (m_visits == 0) {
        reward = playout(agent, dfr);
    } else {
        // not a chance node, pick a maximising action
        action_t a = selectAction(agent);
        // agent.modelUpdate(a) // ??
        return sample(agent, dfr);
    }
    // double v_hat = (reward + m_visits*v_hat)/double(m_visits + 1));
    // XXX: v_hat = m_mean ?
    m_visits++;
    return reward;
}
