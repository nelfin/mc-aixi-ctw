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
	~SearchNode(void);

	// determine the next action to play
	action_t selectAction(Agent &agent) const; // TODO: implement

	// determine the expected reward from this node
	reward_t expectation(void) const { return m_mean; }
    // XXX: this is ridiculous, what is the damn point of typedef-ing this
    // stuff if you aren't even going to stick to your own definitions?

	// perform a sample run through this node and it's children,
	// returning the accumulated reward from this sample run
	reward_t sample(Agent &agent, unsigned int dfr); // TODO: implement

	// number of times the search node has been visited
	visits_t visits(void) const { return m_visits; }

    // return pointer to child corresponding to action/percept
    const SearchNode *child(unsigned int aor) const { return m_child[aor]; }
    // NOTE: action_t and percept_t are both typedef unsigned int, but how do
    // we tell action_t(0) apart from percept_t(0)? Dammit.  I guess chance
    // nodes should always be preceded by an action and vice versa, but
    // goddamit this isn't type-safe. What's the point!?

private:

	bool m_chance_node; // true if this node is a chance node, false otherwise
	double m_mean;      // the expected reward of this node
	visits_t m_visits;  // number of times the search node has been visited

    // how many children should there be?
    SearchNode *m_child[16];
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

SearchNode::SearchNode(bool chance) :
    m_chance_node(chance),
    m_mean(0.0),
    m_visits(0)
{
    // some m_child-ren maybe?
}

SearchNode::~SearchNode(void) {
    for (int i = 0; i < 16; i++) {
        delete m_child[i];
    }
}

// determine the next action to play
action_t SearchNode::selectAction(Agent &agent) const {
    // req: a search tree \Psi
    // req: a history h
    // req: an exploration/exploitation constant C
    return action_t(0);
}

// perform a sample run through this node and it's children,
// returning the accumulated reward from this sample run
reward_t SearchNode::sample(Agent &agent, unsigned int dfr) {
    // req: a search tree \Psi (in agent)
    // req: a history h (also in agent)
    // req: a remaining search horizon m (dfr)
    reward_t reward;
    if (dfr == 0) {
        return reward_t(0.0);
    } else if (m_chance_node) {
        // chance node business
        percept_t ob, r;
        agent.genPerceptAndUpdate(&ob, &r);
        // Create node \Psi(hor) if T(hor) = 0, i.e. it doesn't exist
        if (child(ob) == NULL) {
            m_child[ob] = new SearchNode(false);
        }
        reward = r + m_child[ob]->sample(agent, dfr - 1);
    } else if (m_visits == 0) {
        reward = playout(agent, dfr);
    } else {
        // not a chance node, pick a maximising action
        action_t a = selectAction(agent);
        agent.modelUpdate(a); // should selectAction() do this?
        if (child(a) == NULL) {
            m_child[a] = new SearchNode(true);
        }
        reward = m_child[a]->sample(agent, dfr);
    }
    m_mean = (reward + double(m_visits)*m_mean) /
        (double(m_visits) + 1.0);
    m_visits++;
    return reward;
}
