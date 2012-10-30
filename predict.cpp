#include "predict.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <cmath> // "log" is a BAD idea dude
#include "util.hpp"
#include <limits>
#include <vector>


CTNode::CTNode(void) :
	m_log_prob_est(0),
	m_log_prob_weighted(0)
{
	m_count[0] = 0;
	m_count[1] = 0;
	m_child[0] = NULL;
	m_child[1] = NULL;
}

CTNode::CTNode(const CTNode &other) :
	m_log_prob_est(other.m_log_prob_est),
	m_log_prob_weighted(other.m_log_prob_weighted)
{
	m_count[0] = other.m_count[0];
	m_count[1] = other.m_count[1];
	if (other.m_child[0]) {
		m_child[0] = new CTNode(*other.m_child[0]);
	} else {
		m_child[0] = NULL;
	}
	if (other.m_child[1]) {
		m_child[1] = new CTNode(*other.m_child[1]);
	} else {
		m_child[1] = NULL;
	}
}

CTNode::~CTNode(void) {
	if (m_child[0]) delete m_child[0];
	if (m_child[1]) delete m_child[1];
}

// number of descendants of a node in the context tree
size_t CTNode::size(void) const {

	size_t rval = 1;
	rval += child(false) ? child(false)->size() : 0;
	rval += child(true)  ? child(true)->size()  : 0;
	return rval;
}


// compute the logarithm of the KT-estimator update multiplier
double CTNode::logKTMul(symbol_t sym) const {
	// next-term pseudo-Laplace estimator, doesn't update m_count[]
	int temp = m_count[sym];
	int temp2 = m_count[1 - sym]; // other symbol
	return log(temp + 0.5) - log(temp + temp2 + 1); //=log((temp+0.5) / log(temp+temp2+1))
}


// create a context tree of specified maximum depth
ContextTree::ContextTree(size_t depth){
	assert(depth > 0);
	m_context = new CTNode*[m_depth + 1];
	return;
}

ContextTree::ContextTree(const ContextTree &ct){
	m_depth = ct.m_depth;
	// vector should copy automatically, as it is of simple types
	m_history = ct.m_history;
	m_root = new CTNode(*ct.m_root);
}


// Let's print some REALLY REALLY PRETTY STRINGS, shall we?
std::string CTNode::prettyPrintNode(int depth) {
	std::string answer;
	for (int i = 0; i < depth; i++) {
		answer.append("\t");
	}

	std::string count0 = static_cast<std::ostringstream*>( &(std::ostringstream() << m_count[0]) )->str();
	std::string count1 = static_cast<std::ostringstream*>( &(std::ostringstream() << m_count[1]) )->str();
	std::ostringstream double_to_str_stream;
	double_to_str_stream << "e=" << std::setprecision(8) << m_log_prob_est;
	double_to_str_stream << ", ";
	double_to_str_stream << "w=" << std::setprecision(8) << m_log_prob_weighted;
	std::string log_prob_str = double_to_str_stream.str();
	answer.append(log_prob_str+": (" +
			count0 + "," + count1 + ")\n"); 
	if(m_child[0] != NULL)
		answer.append("0   "+m_child[0]->prettyPrintNode(depth + 1));
	if(m_child[1] != NULL)
		answer.append("1   "+m_child[1]->prettyPrintNode(depth + 1));
	return answer;
}

std::string ContextTree::prettyPrint(void) {
	return m_root->prettyPrintNode(0);
}

// print's the agent's history in the format O R A R A O R ...
std::string ContextTree::printHistory(void) {
	std::string answer;
	history_t::iterator history_iterator = m_history.begin();

	while (history_iterator != m_history.end())
		answer.append(" "+static_cast<std::ostringstream*>( &(std::ostringstream() <<(*history_iterator++)) )->str());
	return answer;
}

ContextTree::~ContextTree(void) {
		m_history.clear();
	delete[] m_context;
	if (m_root)
		delete m_root;
}


// clear the entire context tree
void ContextTree::clear(void) {
	m_history.clear();
	if (m_root) delete m_root;
	m_root = new CTNode();
}

// Recalculate the log weighted probability for this node. Preconditions are:
//  * m_log_prob_est is correct.
//  * logProbWeighted() is correct for each child node.
void CTNode::updateLogProbability(void) {

	// Calculate the log weighted probability. If the current node is a leaf
	// node, this is just the KT estimate. Otherwise it is an even mixture of
	// the KT estimate and the product of the weighted probabilities of the
	// children.
	if (isLeafNode()) {
		m_log_prob_weighted = m_log_prob_est;
	} else {
		// The sum of the log weighted probabilities of the child nodes
		double log_child_prob = 0.0;
		log_child_prob += child(false) ? child(false)->logProbWeighted() : 0.0;
		log_child_prob += child(true) ? child(true)->logProbWeighted() : 0.0;

		// Calculate the log weighted probability. Use the formulation which
		// has the least chance of overflow (see function doc for details).
		double a = std::max(m_log_prob_est, log_child_prob);
		double b = std::min(m_log_prob_est, log_child_prob);
		m_log_prob_weighted = std::log(0.5) + a + std::log(1.0 + std::exp(b - a));
	}
}

// Added to the previous logKT estimate upon observing a new symbol.
weight_t CTNode::logKTMultiplier(const symbol_t symbol) const {
	double numerator = double(m_count[symbol]) + 0.5;
	double denominator = double(visits() + 1);
	return std::log(numerator / denominator);
}

void CTNode::update(symbol_t sym) {
	m_log_prob_est += logKTMultiplier(sym);       // Update KT estimate
	updateLogProbability();                    // Update weighted probability
	m_count[sym]++;                         // Update symbol counts
}

void ContextTree::update(symbol_t sym) {
	// Traverse the tree from leaf to root according to the context. Update the
	// probabilities and symbol counts for each node.
	if (m_history.size() >= m_depth) {
		updateContext();
		for (int i = m_depth; i >= 0; i--) {
			m_context[i]->update(sym);
		}
	}

	// Add symbol to history
	updateHistory(sym);
}


void ContextTree::update(const symbol_list_t &symlist) {
	for (size_t i=0; i < symlist.size(); i++) {
		update(symlist[i]);
	}
}

// Append a symbol to history without updating context tree.
void ContextTree::updateHistory(const symbol_t symbol) {
	m_history.push_back(symbol);
}

// updates the history statistics, without touching the context tree
void ContextTree::updateHistory(const symbol_list_t &symlist) {
	for (size_t i=0; i < symlist.size(); i++) {
		m_history.push_back(symlist[i]);
	}
}

// internal routine to remove a single symbol from the context tree
// TODO: testing
void CTNode::revert(symbol_t sym) {
	m_count[sym]--;                   // Revert symbol count
	if(m_child[sym] && m_child[sym]->visits() == 0) { // Delete unnecessary child node
		delete m_child[sym];
		m_child[sym] = NULL;
	}

	m_log_prob_est -= logKTMultiplier(sym); // Revert KT estimate
	updateLogProbability();              // Revert weighted probability
}

// Get the nodes in the current context
void ContextTree::updateContext(void) {
	assert(m_history.size() >= m_depth);

	// Traverse the tree from root to leaf according to the context. Save the
	// path taken and create new nodes as necessary.
	m_context[0] = m_root;
	CTNode **node = &m_root;
	//symbol_list_t::reverse_iterator symbol_iter = m_history.rbegin();
	for (int i = 1; i <= m_depth; i++) {
		// Address of the pointer to the relevant child node
		node = &((*node)->m_child[m_history.at(i)]);

		// Add node to the path (creating it if it does not exist)
		if (*node == NULL)
			*node = new CTNode();
		m_context[i] = *node;
	}
}

// Revert multiple updates
void ContextTree::revert(const int num_symbols) {
	for(int i = 0; i < num_symbols; i++) {
		revert();
	}
}

// removes the most recently observed symbol from the context tree
void ContextTree::revert(void) {
	// No updates to revert // TODO: maybe this should be an assertion?
	if (m_history.size() == 0)
		return;

	// Get the most recent symbol and delete from history
	const symbol_t symbol = m_history.back();
	m_history.pop_back();

	// Traverse the tree from leaf to root according to the context. Update the
	// probabilities and symbol counts for each node. Delete unnecessary nodes.
	if (m_history.size() >= m_depth) {
		updateContext();
		for (int i = m_depth; i >= 0; i--) {
			m_context[i]->revert(symbol);
		}
	}
}


// shrinks the history down to a former size
void ContextTree::revertHistory(size_t newsize) {
	assert(newsize <= m_history.size());
	while (m_history.size() > newsize) m_history.pop_back();
}



// generate a specified number of random symbols
// distributed according to the context tree statistics
void ContextTree::genRandomSymbols(symbol_list_t &symbols, size_t bits) {
	genRandomSymbolsAndUpdate(symbols, bits);
	// restore the context tree to its original state   // its* ftfy
	for (size_t i=0; i < bits; i++) revert();
}

// guess the next symbol based on our probabilities
symbol_t ContextTree::predictNext() {
	// (via discussion with Mayank)
	// If we don't have enough history then just guess uniformly
	if (historySize() < depth()) {
		return (rand01() < 0.5);
	}
	// Pr(1 | h)
	//  = \frac{Pr(h ^ 1)}{Pr(h)}
	//  = e^{\log{Pr(h ^ 1)} - \log{Pr(h)}}
	//printf("Before:\n");
	//std::cout << this->prettyPrint() << std::endl;
	double pr_h = logBlockProbability();
	update(1);
	double pr_h1 = logBlockProbability();
	revert();
	// How much is acceptable error?
	//printf("After:\n");
	//std::cout << this->prettyPrint() << std::endl;
	assert(fabs(logBlockProbability() - pr_h) < 0.0001);
//	if (pr_h1 > 0.0 || pr_h > 0.0) {
//		printf("%lf %lf\n", pr_h1 ,pr_h);
//		printf("numerical error: Pr(1 | h) = %lf\n", exp(pr_h1 - pr_h));
//		std::cout << this->prettyPrint() << std::endl;
//		std::terminate();
//	}
//	assert(pr_h1 <= 0.0);
//	assert(pr_h <= 0.0);
	return rand01() < exp(pr_h1 - pr_h);
}

// generate a specified number of random symbols distributed according to
// the context tree statistics and update the context tree with the newly
// generated bits
void ContextTree::genRandomSymbolsAndUpdate(symbol_list_t &symbols, size_t bits) {
	// overwrite the whole symlist?
	for (size_t i = 0; i < bits; i++) {
		// generate a symbol
		symbol_t sym = predictNext();
		symbols.push_back(sym);
		update(sym);
	}
}


// the logarithm of the block probability of the whole sequence
double ContextTree::logBlockProbability(void) {
	return m_root->logProbWeighted();
}


// get the n'th most recent history symbol, NULL if doesn't exist
const symbol_t *ContextTree::nthHistorySymbol(size_t n) const {
	return n < m_history.size() ? &m_history[n] : NULL;
}
