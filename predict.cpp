#include "predict.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <cmath> // "log" is a BAD idea dude
#include "util.hpp"
#include <limits>


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
ContextTree::ContextTree(size_t depth) :
	m_root(new CTNode()),
	m_depth(depth)
{ return; }

ContextTree::ContextTree(const ContextTree &ct){
	m_depth = ct.m_depth;
	// vector should copy automatically, as it is of simple types
	m_history = ct.m_history;
	m_root = new CTNode(*ct.m_root);
}


// Printing CTW for debugging
// Let's print some REALLY REALLY PRETTY STRINGS
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
	if (m_root) delete m_root;
}


// clear the entire context tree
void ContextTree::clear(void) {
	m_history.clear();
	if (m_root) delete m_root;
	m_root = new CTNode();
}

void CTNode::update(symbol_t sym, int depth, history_t history) {
	if (depth == 0) {
		// It's a LEAF!
		this->m_log_prob_est += this->logKTMul(sym);
		this->m_count[sym]++;
		this->m_log_prob_weighted = this->m_log_prob_est;
	} else {
		// fill out the tree as we go along
		if (NULL == child(0)) {
			this->m_child[0] = new CTNode();
			this->m_child[1] = new CTNode();
		}
		symbol_t h = history.back();
		history.pop_back();
		this->m_child[h]->update(sym, depth-1, history);
		// the reason this doesn't use child(h) is because
		// apparently "child(h)" isn't const
		this->m_log_prob_est += this->logKTMul(sym);
		this->m_count[sym]++;
		
		//See Equation 12 of IEEE CTW paper
		//Uses identity log(a+c) = log(a) + log(1+exp(log(c) - log(a))
		double x = child(0)->logProbWeighted() + child(1)->logProbWeighted();
		double exponent = x - m_log_prob_est;
		double y;
		if (fabs(exponent) < 42.0) {
			y = log(0.5) + m_log_prob_est + log(1 + exp(exponent));
		} else {
			y = log(0.5) + m_log_prob_est + exponent;
		}
		this->m_log_prob_weighted = y;
		history.push_back(h);
	}
}

void ContextTree::update(symbol_t sym) {
	// Add pre-history
	if (m_history.size() < m_depth) {
		m_history.push_back(sym);
		return;
	}
	m_root->update(sym, m_depth, m_history);
	m_history.push_back(sym); // add the new symbol to the history
}


void ContextTree::update(const symbol_list_t &symlist) {
	for (size_t i=0; i < symlist.size(); i++) {
		update(symlist[i]);
	}
}


// updates the history statistics, without touching the context tree
void ContextTree::updateHistory(const symbol_list_t &symlist) {
	for (size_t i=0; i < symlist.size(); i++) {
		m_history.push_back(symlist[i]);
	}
}

// internal routine to remove a single symbol from the context tree
void CTNode::revert(symbol_t sym, int depth, history_t history) {
	if (depth == 0) {
		this->m_count[sym]--;
		this->m_log_prob_est -= this->logKTMul(sym);
		this->m_log_prob_weighted = this->m_log_prob_est;
	} else {
		// no need to delete nodes just yet
		symbol_t h = history.back();
		history.pop_back();
		this->m_child[h]->revert(sym, depth-1, history);
		this->m_count[sym]--;
		this->m_log_prob_est -= this->logKTMul(sym);
		// If there's nothing under us then we're a leaf again
		if (!child(0)->visits() && !child(1)->visits()) {
			this->m_log_prob_weighted = this->m_log_prob_est;
		} else {
			double x = child(0)->logProbWeighted() + child(1)->logProbWeighted();
			double exponent = x - m_log_prob_est;
			double y;
			if (fabs(exponent) < 42.0) {
				y = log(0.5) + m_log_prob_est + log(1 + exp(exponent));
			} else {
				y = log(0.5) + m_log_prob_est + exponent;
			}
			this->m_log_prob_weighted = y;
		}
		history.push_back(h);
	}
}



// removes the most recently observed symbol from the context tree
void ContextTree::revert(void) {
	symbol_t sym = m_history.back();
	m_history.pop_back();
	if (m_history.size() >= m_depth) {
		m_root->revert(sym, m_depth, m_history);
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
	// restore the context tree to its original state
	for (size_t i=0; i < bits; i++) revert();
}

// guess the next symbol based on our probabilities
symbol_t ContextTree::predictNext() {
	// (via discussion with Mayank)
	// If we don't have enough history then just guess uniformly
	if (historySize() < depth()) {
		return (rand01() < 0.5);
	}
	double pr_h = logBlockProbability();
	update(1);
	double pr_h1 = logBlockProbability();
	revert();
	assert(fabs(logBlockProbability() - pr_h) < 0.0001);
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
