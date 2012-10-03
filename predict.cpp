#include "predict.hpp"

#include <cassert>
#include <cmath> // "log" is a BAD idea dude



CTNode::CTNode(void) :
	m_log_prob_est(0.0),
	m_log_prob_weighted(0.0)
{
	m_count[0] = 0;
	m_count[1] = 0;
	m_child[0] = NULL;
	m_child[1] = NULL;
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
  int temp = m_count[sym] + 1;
  int temp2 = m_count[1 - sym]; // other symbol

  return log((temp + 0.5) / (temp + temp2 + 1));
}


// create a context tree of specified maximum depth
ContextTree::ContextTree(size_t depth) :
	m_root(new CTNode()),
	m_depth(depth)
{ return; }


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

  if (depth == 0 || history.empty()) {
    // It's a LEAF!
    this->m_log_prob_est = this->logKTMul(sym);
    this->m_log_prob_weighted = this->m_log_prob_est;
    this->m_count[sym]++;
  } else {
    symbol_t h = history.back();
    history.pop_back();
    this->m_child[h]->update(sym, depth-1, history);
      // the reason this doesn't use child(h) is because
      // apparently "child(h)" isn't const but it is but it isn't
      // GAHAHAHHHAHAHAHAHAHA
    this->m_log_prob_est = this->logKTMul(sym);
    this->m_log_prob_weighted = log(0.5) +
      log(exp(this->m_log_prob_est) +
	  exp(child(0)->logProbWeighted() +
	      child(1)->logProbWeighted()));
    this->m_count[sym]++;
    history.push_back(h);
  }
}

void ContextTree::update(symbol_t sym) {
  m_root->update(sym, m_depth, m_history); // cheating ;)
  // history.push_back(sym); // add the new symbol to the history, do we need to do this?
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


// removes the most recently observed symbol from the context tree
void ContextTree::revert(void) {
	// TODO: implement
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


// generate a specified number of random symbols distributed according to
// the context tree statistics and update the context tree with the newly
// generated bits
void ContextTree::genRandomSymbolsAndUpdate(symbol_list_t &symbols, size_t bits) {
	// TODO: implement
}


// the logarithm of the block probability of the whole sequence
double ContextTree::logBlockProbability(void) {
    return m_root->logProbWeighted();
}


// get the n'th most recent history symbol, NULL if doesn't exist
const symbol_t *ContextTree::nthHistorySymbol(size_t n) const {
    return n < m_history.size() ? &m_history[n] : NULL;
}