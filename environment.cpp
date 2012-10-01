#include "environment.hpp"

#include <cassert>

#include "util.hpp"

CoinFlip::CoinFlip(options_t &options) {
	// Determine the probability of the coin landing on heads
	p = 1.0;
	if (options.count("coin-flip-p") > 0) {
		strExtract(options["coin-flip-p"], p);
	}
	assert(0.0 <= p);
	assert(p <= 1.0);

	// Set up the initial observation
	m_observation = rand01() < p ? 1 : 0;
	m_reward = 0;
}

// Observes 1 (heads) with probability p and 0 (tails) with probability 1 - p.
// Observations are independent of the agent's actions. Gives a reward of 1 if
// the agent correctly predicts the next observation and 0 otherwise.
void CoinFlip::performAction(action_t action) {
	m_observation = rand01() < p ? 1 : 0;
	m_reward = action == m_observation ? 1 : 0;
}

//  Grid World Environment
#define SIZE 4
#define DESTX 4
#define DESTY 4

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define NOTHING 0

GridWorld::GridWorld(options_t &options) {
	
	x = randRange(SIZE+1);
	y = randRange(SIZE+1);

	// Set up the initial observation
	m_observation = 0;
	m_reward = 0;
}

void GridWorld::performAction(action_t action) {

	m_reward = 0;
	m_observation = NOTHING;
	
	if (x == DESTX && y == DESTY){
		m_reward = 1;
		x = randRange(SIZE+1);
		y = randRange(SIZE+1);
	}
	
	switch (action){
		case UP:
			x++;
			break;
		case RIGHT:
			x++;
			y++;
			break;
		case DOWN:
			y--;
			break;
		case LEFT:
			y--;
			x--;
			break;
		default:
			printf("Error: Unhandled action case");
	}
	
	if (x>SIZE){
		x = SIZE;
	} else if (x<0){
		x = 0;
	} else if (y>SIZE){
		y = SIZE;
	} else if (y<0){
		y = 0;
	}
	
	log << "position: " << x << "," << y << std::endl;
	
}
