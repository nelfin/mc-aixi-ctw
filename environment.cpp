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
	
	m_x = randRange(SIZE+1);
	m_y = randRange(SIZE+1);

	// Set up the initial observation
	m_observation = 0;
	m_reward = 0;
}

void GridWorld::performAction(action_t action) {

	m_reward = 0;
	m_observation = NOTHING;
	
	if (m_x == DESTX && m_y == DESTY){
		m_reward = 1;
		m_x = randRange(SIZE+1);
		m_y = randRange(SIZE+1);
	}
	
	switch (action){
		case UP:
			m_x++;
			break;
		case RIGHT:
			m_x++;
			m_y++;
			break;
		case DOWN:
			m_y--;
			break;
		case LEFT:
			m_y--;
			m_x--;
			break;
		default:
			printf("Error: Unhandled action case");
	}
	
	if (m_x>SIZE){
		m_x = SIZE;
	} else if (m_x<0){
		m_x = 0;
	} else if (m_y>SIZE){
		m_y = SIZE;
	} else if (m_y<0){
		m_y = 0;
	}
	
	log << "position: " << m_x << "," << m_y << std::endl;
	
}

//biased rock paper scissors Environment
//Keeps the m_reward value at zero and uses m_signed_reward instead
#define ROCK 0
#define SCISSORS 1
#define PAPER 2

RPS::RPS(options_t &options) {
	// Set up the initial observation
	m_observation = 0;
	m_signed_reward = 0;
	
	m_previous_rock_win = 0;
}

void RPS::performAction(action_t action) {
	if(m_previous_rock_win){
		m_observation = ROCK;
	}else{
		m_observation = randRange(3);
	}
	
	if((action+1)%3 == m_observation){
		m_signed_reward = 1;
		log << "result: Agent won with " << action << " (reward " << m_signed_reward<< ")" << std::endl;
	}else if (action == m_observation){
		m_signed_reward = 0;
		log << "result: Draw with " << action << " (reward " << m_signed_reward<< ")" << std::endl;
	}else{
		m_signed_reward = -1;
		log << "result: Environment won with " << m_observation << " (reward " << m_signed_reward << ")" << std::endl;
	}
	//log << "played: " << m_observation << " vs agent's " << action << std::endl;
	
}

//Pacman environment
#define DIMX 17
#define DIMY 17
Pacman::Pacman(options_t &options) {
	// Set up the initial observation
	map = malloc(sizeof(char)*DIMX*DIMY);
	map[x][y]
	m_observation = 0;
	m_signed_reward = 0;
}

void Pacman::performAction(action_t action) {
	
}
