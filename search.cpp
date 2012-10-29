#include "search.hpp"

#include "agent.hpp"
#include "util.hpp"

#include <map>
#include <cmath>
#include <cassert>


typedef unsigned long long visits_t;

// search options
static const visits_t	 MinVisitsBeforeExpansion = 1;
static const unsigned int MaxDistanceFromRoot  = 100;
//static size_t			 MaxSearchNodes;

// UCB bound constants
static const double C = 1.0;

// contains information about a single "state"
class SearchNode {

public:

	SearchNode();
	SearchNode(bool is_chance_node);
	~SearchNode(void);

	// determine the next action to play
	action_t selectAction(Agent &agent) const;

	// determine the expected reward from this node
	reward_t expectation(void) const { return m_mean; }
	// XXX: this is ridiculous, what is the damn point of typedef-ing this
	// stuff if you aren't even going to stick to your own definitions?

	// perform a sample run through this node and it's children,
	// returning the accumulated reward from this sample run
	reward_t sample(Agent &agent, unsigned int dfr);

	// number of times the search node has been visited
	visits_t visits(void) const { return m_visits; }

	// return pointer to child corresponding to action/percept
	const SearchNode *child(unsigned int aor) const;

private:

	bool m_chance_node; // true if this node is a chance node, false otherwise
	double m_mean;	  // the expected reward of this node
	visits_t m_visits;  // number of times the search node has been visited

	// NOTE: action_t and percept_t are both typedef unsigned int, but how do
	// we tell action_t(0) apart from percept_t(0)? Dammit.  I guess chance
	// nodes should always be preceded by an action and vice versa, but
	// goddamit this isn't type-safe. What's the point!?
	typedef std::map<unsigned int, SearchNode> m_child_t;
	m_child_t m_child;
};

// simulate a path through a hypothetical future for the agent within it's
// internal model of the world, returning the accumulated reward.
static reward_t playout(Agent &agent, unsigned int playout_len) {
	ModelUndo *mu = new ModelUndo(agent); 
	reward_t reward = 0.0;
	action_t a;
	percept_t ob, r;
	for (unsigned int i = 0; i < playout_len; i++) {
		a = agent.genRandomAction();
		agent.modelUpdate(a);
		agent.genPerceptAndUpdate(&ob, &r);
		reward += reward_t(r);
	}
	agent.modelRevert(*mu);
	return reward;
}

// determine the best action by searching ahead using MCTS
extern action_t search(Agent &agent) {
	SearchNode *root = new SearchNode(false);
	const int simulations = agent.numSimulations();
	for (int i = 0; i < simulations; i++) {
		// should we be throwing away the accumulated reward of the run that
		// sample generates?
		root->sample(agent, agent.horizon());
	}
	action_t best_action = NULL;
	double best_score = -1.0;
	for (action_t a = 0; a < agent.numActions(); a++) {
		const SearchNode *ha = root->child(a);
		if (NULL == ha) {
			// Why was this node not explored is a more important question.
			// Should this be an assert?
			continue;
		}
		double score = ha->expectation();
		if (score > best_score) {
			best_score = score;
			best_action = a;
		}
	}
	// Hopefully we're not leaking too much memory.
	delete root;
	if (best_score < 0.0) {
		return agent.genRandomAction();
	} else {
		return best_action;
	}
}

SearchNode::SearchNode(bool chance) :
	m_chance_node(chance),
	m_mean(0.0),
	m_visits(0)
{
	// some m_child-ren maybe?
}

SearchNode::SearchNode() :
	m_chance_node(false),
	m_mean(0.0),
	m_visits(0)
{
}

SearchNode::~SearchNode(void) {
	// do I have to delete m_child?
}

// return pointer to child corresponding to action/percept
const SearchNode* SearchNode::child(unsigned int aor) const {
	m_child_t::const_iterator it = m_child.find(aor);
	if (it == m_child.end()) {
		return NULL;
	}
	return &(it->second);
}

// determine the next action to play
action_t SearchNode::selectAction(Agent &agent) const {
	// req: a search tree \Psi
	// req: a history h
	// req: an exploration/exploitation constant C
	//
	// This is a decision node, therefore the children of this node should be
	// indexed by actions. We iterate through them to find the ones which have
	// not been expanded (or somehow expanded but not visited).
	const double norm_factor = double(agent.horizon() * agent.maxReward());
	double unexplored_score = -1.0;
	double explored_score = -1.0;
	action_t best_action = NULL;
	int num_actions = agent.numActions();
	action_t *unexplored = new action_t[num_actions];
	action_t *best = new action_t[num_actions];
	int unexploredactions = 0;
	int bestactions = 0;

	for (action_t a = 0; a < num_actions; a++) {
		const SearchNode *ha = child(a);
		if (NULL == ha || ha->visits() == 0) {
			// unexplored
			unexplored[unexploredactions] = a;
			unexploredactions++;
		} 
	}
	
	if (unexploredactions != 0){
		best_action = unexplored[randRange(0, unexploredactions)];	
	}
	else { // All actions have been explored
		for (action_t a = 0; a < num_actions; a++) {
			const SearchNode *ha = child(a);
			double win_value = ha->expectation() / norm_factor;
			double ucb_bound = C * sqrt(log(visits()) / ha->visits());
			double score = win_value + ucb_bound;
			if (score > explored_score) {
				explored_score = score;
				bestactions = 0;
				best[bestactions] = a;
			} else if (score == explored_score) {
				bestactions++;
				best[bestactions] = a;
			}
		}
		if (bestactions > 0)
			best_action = best[randRange(0, bestactions+1)];
		else
			best_action = best[0];
	}

	delete[] unexplored;
	delete[] best;

	return best_action;
}

// perform a sample run through this node and it's children,
// returning the accumulated reward from this sample run
reward_t SearchNode::sample(Agent &agent, unsigned int dfr) {
  
	// Savepoint
	ModelUndo *mu = new ModelUndo(agent);  
  
	// req: a search tree \Psi (in agent)
	// req: a history h (also in agent)
	// req: a remaining search horizon m (dfr)
	reward_t reward;
	if (dfr == 0) {
		return reward_t(0.0);
	} else if (m_chance_node) { // Is this set properly?
		// chance node business
		// Generates (o,r) from the ctw given h
		percept_t ob, r;
		agent.genPerceptAndUpdate(&ob, &r);
		// Create node \Psi(hor) if T(hor) = 0, i.e. it doesn't exist
		if (child(ob) == NULL) {
			m_child[ob] = new SearchNode(false);
		}
		reward = r + m_child[ob].sample(agent, dfr - 1);
	} else if (m_visits == 0) {
		reward = playout(agent, dfr);
	} else {
		// not a chance node, pick a maximising action
		action_t a = selectAction(agent);
		agent.modelUpdate(a); // should selectAction() do this?
		if (child(a) == NULL) {
			m_child[a] = new SearchNode(true);
		}
		reward = m_child[a].sample(agent, dfr);
	}

	// Back propagation:
	m_mean = (reward + double(m_visits)*m_mean) / (double(m_visits) + 1.0);
	m_visits++;

	// Restore from savepoint
	agent.modelRevert(*mu);

	// Return reward
	return reward;
}
