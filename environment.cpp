#include "environment.hpp"

#include <cassert>

#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

#include "util.hpp"
#include "environment.hpp"

// Coin Flip

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


/* Tiger:

actions
	open left - 0
	open right - 1
	listen - 2

observation
	tiger behind left - 0
	tiger behind right - 1
	no observation - 2

m_gold_door
	0 - gold behind left
	1 - gold behind right
*/

Tiger::Tiger(options_t &options) {
	p = 1.0;
	if (options.count("left-door-p") > 0) {
		strExtract(options["left-door-p"], p);
	}
	assert(0.0 <= p);
	assert(p <= 1.0);
	m_listen_chance = 0.85;
	if (options.count("listen-p") > 0) {
		strExtract(options["listen-p"], m_listen_chance);
	}
	assert(0.0 <= m_listen_chance);
	assert(m_listen_chance <= 1.0);
	
	m_gold_door = rand01() < p ? 1 : 0;
	m_signed_reward = 0;
	m_observation = 2;
	//behindLeftDoor = true;
}
void Tiger::performAction(action_t action) {

	if (action == 2) {								// Listen
		m_signed_reward = -1;
		if (rand01() < m_listen_chance) {			//Hears the tiger behind the correct door.
			m_observation = !m_gold_door;
		} else {									//Hears the tiger behind the incorrect door.
			m_observation = m_gold_door;
		}
	} else {
		if (action == m_gold_door) { // Chooses the door with the gold
			m_signed_reward = 10;
		} else {
			m_signed_reward = -100;
		}
		m_observation = 2;
		m_gold_door = rand01() < p ? 1 : 0;
	}
}

//  Grid World Environment
#define SIZE 4
#define DESTX 4
#define DESTY 4

#define GUP 0
#define GRIGHT 1
#define GDOWN 2
#define GLEFT 3

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
		case GUP:
			m_x++;
			break;
		case GRIGHT:
			m_x++;
			m_y++;
			break;
		case GDOWN:
			m_y--;
			break;
		case GLEFT:
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
	
	logFile << "position: " << m_x << "," << m_y << std::endl;
	
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
		logFile << "result: Agent won with " << action << " (reward " << m_signed_reward<< ")" << std::endl;
	}else if (action == m_observation){
		m_signed_reward = 0;
		logFile << "result: Draw with " << action << " (reward " << m_signed_reward<< ")" << std::endl;
	}else{
		m_signed_reward = -1;
		logFile << "result: Environment won with " << m_observation << " (reward " << m_signed_reward << ")" << std::endl;
	}
	//logFile << "played: " << m_observation << " vs agent's " << action << std::endl;
	
}

/* Kuhn Poker environment:

actions
	Pass: 0
	Bet: 1

observation (Player Card, Opponent Action)
	Player Card
		Jack: 0
		Queen: 1
		King: 2
	Opponent Action
		Pass: 0
		Bet: 1

reward: (Pot - Investment)

NOTE: 
Add an alternate environment which includes as an observation the 
outcome of the previous round (Opponent Reveal, Player Card, Opponent Action)
	Opponent Reveal
		Jack: 0
		Queen: 1
		King: 2
		Fold: 3 (no showdown)	
*/

#define JACK 0
#define QUEEN 1
#define KING 2

KuhnPoker::KuhnPoker(options_t &options) {
	gamma = 0.5;
	if (options.count("gamma") > 0) {
		strExtract(options["gamma"], gamma);
	}
	alpha =  gamma / 3.0;
	beta = (1+gamma) / 3.0;	

	dealCards();
	
	// Implement strategy for opponent (bet|pass given m_opponent_card)
	m_opponent_action = getFirstNashAction();
	std::cout << "Opponent chose action "<< m_opponent_action <<std::endl;
	
	// m_observation = (m_player_card, m_opponent_action);
	m_observation = 2*m_player_card + m_opponent_action; // Does this operation seem correct?
	m_signed_reward = 0;
	
}

int KuhnPoker::getFirstNashAction() {
    if(m_opponent_card==JACK){
        return rand01() < alpha ? 1 : 0;
    }
    if(m_opponent_card==QUEEN){
        return 0;
    }
    if(m_opponent_card==KING){
        return rand01() < gamma ? 1 : 0;
    }
}

int KuhnPoker::getSecondNashAction() {
    if(m_opponent_card==JACK){
        return 0;
    }
    if(m_opponent_card==QUEEN){
        return rand01() < beta ? 1 : 0;
    }
    if(m_opponent_card==KING){
        return 1;
    }
}

void KuhnPoker::dealCards() {
    m_opponent_card = randRange(3); 		// Random number 0 = Jack, 1 = Queen, 2 = King 
	if (randRange(2) == 1){	// 50-50 chance of getting the either of the remaining cards
		m_player_card =  (m_opponent_card + 1) % 3; 	
	} else {
		m_player_card =  (m_opponent_card + 2) % 3; 	
	}
	std::cout << "Player has card "<< m_player_card <<std::endl;
}

void KuhnPoker::performAction(action_t action) {

	// Initialise temporary variables
	m_win_flag = false;
	m_pot = 2;
	m_investment = 1;
	
	// Action is either bet (1) or pass (0)
	if (m_opponent_action == 0){ // opponent passes
		if (action == 0){ // player pass (investment = blind = 1)
			// SHOWDOWN
			m_win_flag = (m_player_card > m_opponent_card);
		} else { // player bet (investment = 2)
			m_pot++;
			m_investment++;
			// Opponent gets to bet
			int opponent_second_action = getSecondNashAction();
			std::cout << "Opponent's second action was "<< opponent_second_action <<std::endl;
			if (opponent_second_action == 0) { // Opponent passes again
				// Player wins - NO SHOWDOWN
				m_win_flag = true;
			} else { // Opponent bets
				m_pot++;
				// SHOWDOWN
				m_win_flag = (m_player_card > m_opponent_card);
			}
		}	
	} else { // Opponent bets
		m_pot++;
		if (action == 0){ // player pass (investment = blind = 1)
			// Opponent wins - NO SHOWDOWN
			m_win_flag = false;
		} else { // player bet
			m_pot++;
			m_investment++;
			// SHOWDOWN
			m_win_flag = (m_player_card > m_opponent_card);
		}	
	}

	// REWARD
	if (m_win_flag){ // Player wins
		m_signed_reward = m_pot - m_investment;
		std::cout << "Opponent had card "<< m_opponent_card << ", player had card " << m_player_card << " so player won" <<std::endl;
	} else {
		m_signed_reward = 0 - m_investment;
		std::cout << "Opponent had card "<< m_opponent_card << ", player had card " << m_player_card << " so opponent won" <<std::endl;
	}
	std::cout << "Player reward " << m_signed_reward <<std::endl;
	
	dealCards();
	
	// Implement strategy for opponent (bet|pass given m_opponent_card)
	m_opponent_action = getFirstNashAction(); // Randomly chooses to pass (0) or bet (1)
	std::cout << "Opponent chose action "<< m_opponent_action <<std::endl;
	// OBSERVE
	// Player Card 		= m_player_card
	// Opponent Action 	= m_opponent_action
	// m_observation = (m_player_card, m_opponent_action);
	m_observation = 2*m_player_card + m_opponent_action; // Does this operation seem correct?
	
}

//Pacman environment

Pacman::Pacman(options_t &options) {
	reset();
	// Initial observation setup
	m_observation = setObservation();	
	m_signed_reward = 0;
}

//Debug method to show the map
//Also fun
void Pacman::printMap(){
	
	printf("\n");
	for (int cury = 0; cury < dimy; cury++){
		
		for (int curx = 0; curx < dimx; curx++){
			switch (map[curx][cury]){
				case WALL:
					printf("\033[42m ");
					break;
				case EMPTY:
					printf("\033[47m ");
					break;
				case FOOD:
					printf("\033[46m ");
					break;
				case PACMAN:
					if (power){
						printf("\033[45m ");
					} else {
						printf("\033[43m ");
					}
					break;
				case GHOST:
				case PILLANDGHOST:
				case FOODANDGHOST:
					printf("\033[41m ");
					break;
				case PILL:
					printf("\033[44m ");
					break;
				default:
					printf("%d", map[curx][cury]);
			}
		}
		printf("\033[0m\n");
	}
}

void Pacman::reset(){
	//Read map in from file
	std::ifstream pacmap ("pacman.map");
	std::string nextline;
	
	getline(pacmap,nextline);
	dimx = atoi(nextline.c_str());
	getline(pacmap,nextline);
	dimy = atoi(nextline.c_str());
	getline(pacmap,nextline);
	numghosts = atoi(nextline.c_str());
	
	// Allocate and clear memory for map
	map = new tile_t*[dimx];
	for (int i = 0; i < dimx; i++){
		map[i] = new tile_t[dimy];
	}
	// Allocate memory for ghosts
	ghosts = new ghostinfo_t[numghosts];
	
	//Read in map tiles
	foodleft = 0;
	powertime = 0;
	int ghostcount = 0;
	char curtile = 0;
	
	for (int cury = 0; cury < dimy; cury++){
		getline(pacmap,nextline);
		
		for (int curx = 0; curx < dimx; curx++){
			curtile = nextline[curx];
			
			// Convert from char to tile_t
			map[curx][cury] = (tile_t) (curtile - '0');
			if (map[curx][cury] == EMPTY){
				//50% chance to be food in empty square
				if (randRange(2) == 1){
					map[curx][cury] = FOOD;
					foodleft++;
				}
			}
			else if (map[curx][cury] == PACMAN){
				pacman.x = curx;
				pacman.y = cury;
			}
			else if (map[curx][cury] == GHOST){
			    //Initialise ghost information
				ghosts[ghostcount].pos.x = curx;
				ghosts[ghostcount].pos.y = cury;
				ghosts[ghostcount].pursue = 0;
				ghosts[ghostcount].cooldown = 0;
				ghosts[ghostcount].alive = true;
				ghostcount++;
			}
		}
	}
	
	printMap();
	
	//Other initial variables
	power = false;
}


//walls
#define W_UP 0
#define W_RIGHT 1
#define W_DOWN 2
#define W_LEFT 3
//food line of sight
#define F_UP 4
#define F_RIGHT 5
#define F_DOWN 6
#define F_LEFT 7
//pill
#define POWERED 8
//smells
#define S_2 9
#define S_3 10
#define S_4 11
//ghost line of sight
#define G_UP 12
#define G_RIGHT 13
#define G_DOWN 14
#define G_LEFT 15

percept_t Pacman::setObservation(){
	short observation = 0;
	
	//walls
	observation = observation | ((validSquare(makeMove(pacman,UP)) ? 1 : 0) << W_UP);
	observation = observation | ((validSquare(makeMove(pacman,RIGHT)) ? 1 : 0) << W_RIGHT);
	observation = observation | ((validSquare(makeMove(pacman,DOWN)) ? 1 : 0) << W_DOWN);
	observation = observation | ((validSquare(makeMove(pacman,LEFT)) ? 1 : 0) << W_LEFT);
	
	//food line of sight
	observation = observation | ((lineOfSight(pacman,UP,FOOD) ? 1 : 0) << W_UP);
	observation = observation | ((lineOfSight(pacman,RIGHT,FOOD) ? 1 : 0) << W_RIGHT);
	observation = observation | ((lineOfSight(pacman,DOWN,FOOD) ? 1 : 0) << W_DOWN);
	observation = observation | ((lineOfSight(pacman,LEFT,FOOD) ? 1 : 0) << W_LEFT);
	
	//pill
	observation = observation | ((power ? 1 : 0) << POWERED);
	
	//smells (for distances 2,3,4)
	if (manhattanSearch(pacman,NONE,NONE, 2, FOOD)){
		//push up binary 111
		observation = observation | (5 << S_2);
	} else if (manhattanSearch(pacman,NONE,NONE, 3, FOOD)){
		//push up binary 11
		observation = observation | (3 << S_3);
	} else if (manhattanSearch(pacman,NONE,NONE, 4, FOOD)){
		//push up binary 1
		observation = observation | (1 << S_4);		
	}
	
	//ghost line of sight
	observation = observation | ((lineOfSight(pacman,UP,GHOST) ? 1 : 0) << G_UP);
	observation = observation | ((lineOfSight(pacman,RIGHT,GHOST) ? 1 : 0) << G_RIGHT);
	observation = observation | ((lineOfSight(pacman,DOWN,GHOST) ? 1 : 0) << G_DOWN);
	observation = observation | ((lineOfSight(pacman,LEFT,GHOST) ? 1 : 0) << G_LEFT);
	
	return (percept_t) observation;
}

Pacman::direction_t Pacman::manhattanSearch(coord_t curcoord, direction_t camefrom, direction_t wentto, int dist, tile_t seeking){
	//check if found it
	if (testSquare(curcoord,seeking)){
	    return wentto;
	}
	//check if max depth
	if (dist == 0){
		return NONE;
	}
	
	direction_t searchresult;
	
	//Check each direction, if there is a move which sees the target, return it
	if (validSquare(makeMove(curcoord, UP)) && (camefrom != UP)){
		searchresult = manhattanSearch(makeMove(curcoord, UP), DOWN, UP, dist-1, seeking);
		if (searchresult != NONE){
			return UP;
		}
	}
	if (validSquare(makeMove(curcoord, RIGHT)) && camefrom != RIGHT){
		searchresult = manhattanSearch(makeMove(curcoord, RIGHT), LEFT, RIGHT, dist-1, seeking);		
		if (searchresult != NONE){
			return RIGHT;
		}
	}
	if (validSquare(makeMove(curcoord, DOWN)) && camefrom != DOWN){
		searchresult = manhattanSearch(makeMove(curcoord, DOWN), UP, DOWN, dist-1, seeking);		
		if (searchresult != NONE){
			return DOWN;
		}
	}
	if (validSquare(makeMove(curcoord, LEFT)) && camefrom != LEFT){
		searchresult = manhattanSearch(makeMove(curcoord, LEFT), RIGHT, LEFT, dist-1, seeking);
		if (searchresult != NONE){
			return LEFT;
		}
	}
	return NONE;
}

bool Pacman::lineOfSight(coord_t curcoord, direction_t direction, tile_t seeking){
	coord_t sightcoord;
	sightcoord.x = curcoord.x;
	sightcoord.y = curcoord.y;
	
	//Keep moving in direction until we see what we're after or hit a wall
	sightcoord = makeMove(sightcoord,direction);	
	while (validSquare(sightcoord)){
		if (testSquare(sightcoord, seeking))
			return true;
		
		sightcoord = makeMove(sightcoord,direction);
	}
	return false;
}

Pacman::coord_t Pacman::makeMove(coord_t coord, direction_t move){
	coord_t result;
	//Move and return coord
	switch (move){
		case UP:
			result.x = coord.x;
			result.y = coord.y-1;
			break;
		case RIGHT:
			result.x = coord.x+1;
			result.y = coord.y;
			break;
		case DOWN:
			result.x = coord.x;
			result.y = coord.y+1;
			break;
		case LEFT:
			result.x = coord.x-1;
			result.y = coord.y;
			break;
		default:
			printf("Error: makeMove unhandled case");
			assert(false);
	}
	
	//Modulus wraps numbers which are too large
	result.x = result.x % dimx;
	//Add dimension to disallow negative numbers
	//(Modulus does not always take care of this in C)
	if (result.x < 0)
		result.x = result.x + dimx;
	
	result.y = result.y % dimy;
	if (result.y < 0)
		result.y = result.y + dimy;
	
	return result;
}

//Valid/walkable square if not wall
bool Pacman::validSquare(coord_t coord){
	return map[coord.x][coord.y] != WALL;
}

//Test if square contains the object sought
bool Pacman::testSquare(coord_t coord, tile_t seeking){
	switch(seeking){
		case(FOOD):
			return (map[coord.x][coord.y] == FOOD || map[coord.x][coord.y] == FOODANDGHOST);
		case(GHOST):
			return (map[coord.x][coord.y] == GHOST || map[coord.x][coord.y] == FOODANDGHOST
				|| map[coord.x][coord.y] == PILLANDGHOST);
		case(PILL):
			return (map[coord.x][coord.y] == PILL || map[coord.x][coord.y] == PILLANDGHOST);
		case(PACMAN):
			return (map[coord.x][coord.y] == PACMAN);
	}
}

//Reward constants
#define COMPLETE 100
#define FOODREWARD 10
#define GHOSTEATREWARD 30
#define MOVEREWARD -1
#define WALLREWARD -10
#define GHOSTREWARD -50

//Behavioural constants
#define SHORT_DURATION 10
#define COOLDOWN 10
#define AGRESSIVEDIST 5
#define PILLDURATION 40
void Pacman::performAction(action_t action) {
	direction_t newaction = (direction_t) action;
	m_signed_reward = 0;
	
	//Make pacman's move
	coord_t newsquare = makeMove(pacman,newaction);
	if (!(validSquare(newsquare))){
		m_signed_reward += WALLREWARD;
	} else {
	    //Perform the move
		m_signed_reward += MOVEREWARD;
		map[pacman.x][pacman.y] = EMPTY;
		pacman.x = newsquare.x;
		pacman.y = newsquare.y;
		
		//Check landing square
		
		//Running into a ghost
		if(testSquare(newsquare, GHOST)){
			if (power){
				//Pacman kills ghost
				for (int i = 0; i < numghosts; i++){
					if (ghosts[i].alive && ghosts[i].pos.x == pacman.x && ghosts[i].pos.y == pacman.y){
						ghosts[i].alive = false;
				        m_signed_reward += GHOSTEATREWARD;
					}
				}
			} else {
				//Ghost kills pacman
				m_signed_reward += GHOSTREWARD;
				reset();
				return;
			}
		}
		
		//Eating food
		if(testSquare(newsquare, FOOD)){
			m_signed_reward += FOODREWARD;
			foodleft--;
			if (foodleft == 0){
				//Eaten everything, gameover
			    m_signed_reward += COMPLETE;
				reset();
				return;
			}
		//Eating power pill
		} else if(testSquare(newsquare, PILL)){
			power = true;
			powertime = PILLDURATION;
		}
		
		map[pacman.x][pacman.y] = PACMAN;
	}
	
	//Make ghost moves
	for (int i = 0; i < numghosts; i++){
		if (ghosts[i].alive){
			
			direction_t ghostmove = NONE;
			
			//If not on cooldown
			if (ghosts[i].cooldown == 0){
				//Search for pacman
				//Ghostmove will become NONE or a move towards pacman
				ghostmove = manhattanSearch(ghosts[i].pos, NONE, NONE, AGRESSIVEDIST, PACMAN);
				
				//Manage pursuit time and cooldown
				if (ghosts[i].pursue > 0){
					ghosts[i].pursue--;
					if (ghosts[i].pursue == 0){
						ghosts[i].cooldown = COOLDOWN;
					}
				} else if (ghostmove != NONE){
					ghosts[i].pursue = SHORT_DURATION;
				}
			} else ghosts[i].cooldown--;
				
			//If can't smell pacman
			if (ghostmove == NONE){
				//Make a random move
				ghostmove = randomMove(ghosts[i].pos);
			}
			//Advance position
			coord_t newcoord = makeMove(ghosts[i].pos, ghostmove);
			
			//Check if ghost lands on pacman
			if (newcoord.x == pacman.x && newcoord.y == pacman.y){
				if (power){
					//Pacman kills ghost
					ghosts[i].alive = false;
				} else {
					//Pacman dies
					m_signed_reward += GHOSTREWARD;
					reset();
					return;
				}
			}
			//Perform move
			ghostMove(ghosts[i].pos, newcoord);
			//Update ghost info
			ghosts[i].pos.x = newcoord.x;
			ghosts[i].pos.y = newcoord.y;
		}
	}
	
	//End of action power ticks
	if (power){
		powertime--;
		if (powertime == 0)
			power = false;
	}
	
	printMap();
	m_observation = setObservation();	
}

Pacman::direction_t Pacman::randomMove(coord_t coord){
	direction_t possiblemoves[4];
	int nummoves = 0;
	
	//Add all valid directions to an array
	if (validSquare(makeMove(coord,UP))){
		possiblemoves[nummoves] = UP;
		nummoves++;
	}
	if (validSquare(makeMove(coord,RIGHT))){
		possiblemoves[nummoves] = RIGHT;
		nummoves++;
		
	}
	if (validSquare(makeMove(coord,DOWN))){
		possiblemoves[nummoves] = DOWN;
		nummoves++;
		
	}
	if (validSquare(makeMove(coord,LEFT))){
		possiblemoves[nummoves] = LEFT;
		nummoves++;
		
	}
	
	//Returns a random filled element of the array
	return possiblemoves[randRange(nummoves)];
}

void Pacman::ghostMove(coord_t from, coord_t to){
	switch(map[from.x][from.y]){
		case GHOST:
			map[from.x][from.y] = EMPTY;
			break;
		case PILLANDGHOST:
			map[from.x][from.y] = PILL;
			break;
		case FOODANDGHOST:
			map[from.x][from.y] = FOOD;
			break;
		case WALL:
			printf("Error: Trying to move ghost out of square where there is a wall!\n");
			printf("%d, %d has a %d\n", from.x, from.y, map[from.x][from.y]);
		case EMPTY:
		default:
			//unlikely but possible that two ghosts occupied the same square
			//do nothing
			break;
	}
	
	switch(map[to.x][to.y]){
		case PILL:
			map[to.x][to.y] = PILLANDGHOST;
			break;
		case FOOD:
			map[to.x][to.y] = FOODANDGHOST;
			break;
		case FOODANDGHOST:
			map[to.x][to.y] = FOODANDGHOST;
			break;
		case PILLANDGHOST:
			map[to.x][to.y] = PILLANDGHOST;
			break;
		case PACMAN:
			//if we reach this spot, this ghost must be dead (otherwise the game would've ended)
			break;
		case WALL:
			printf("Error: Ghost trying to move into a wall!\n");
			printf("%d, %d has a %d\n", to.x, to.y, map[to.x][to.y]);
		default:
			map[to.x][to.y] = GHOST;
	}
}

/* 
Composite Environments
*/

// Generic Composite Constructor
Composite::Composite(options_t &options) {

	// Extract environment sequence and changeover times
	int i;
	for (i = 0; options.count(environmentCode(i)) > 0 && options.count(startCode(i)) > 0; i++){
		strExtract(options[environmentCode(i)], m_environment[i]);
		strExtract(options[startCode(i)], m_start[i]);
	}
	m_last_environment = i - 1;

	// The options must be stored for use in dynamically initialising the second environment
	m_options = options;

	// Initialise the first environment
	m_current_environment = 0;
	initialise(m_options, m_environment[m_current_environment]);

	// Initialise the current cycle number - for calculating changeover
	m_current_cycle = 1;

}

// These two functions should use integer to string conversion instead 
// but C++ is bogus
// The arrays are defined statically too, so whatever
std::string Composite::environmentCode(int i){
	switch(i){
	case 0:
		return "environment1";
		break;
	case 1:
		return "environment2";
		break;
	case 2:
		return "environment3";
		break;
	case 3:
		return "environment4";
		break;
	case 4:
		return "environment5";
		break;
	case 5:
		return "environment6";
		break;
	case 6:
		return "environment7";
		break;
	case 7:
		return "environment8";
		break;
	case 8:
		return "environment9";
		break;
	case 9:
		return "environment10";
		break;
	default:
		return "do_not_use_this_option";
		break;
	}
}

std::string Composite::startCode(int i){
	switch(i){
	case 0:
		return "start1";
		break;
	case 1:
		return "start2";
		break;
	case 2:
		return "start3";
		break;
	case 3:
		return "start4";
		break;
	case 4:
		return "start5";
		break;
	case 5:
		return "start6";
		break;
	case 6:
		return "start7";
		break;
	case 7:
		return "start8";
		break;
	case 8:
		return "start9";
		break;
	case 9:
		return "start10";
		break;
	default:
		return "do_not_use_this_option";
		break;
	}
}

// Generic Composite Action Resolution
void Composite::performAction(action_t action){

		// Resolve action for first environment
		resolveAction(action, m_environment[m_current_environment]);

		// Check for changeover in next cycle
		if (m_current_environment != m_last_environment) { // Cannot change if on the last environment
			m_current_cycle++; // Keep track of the current cycle until the last environment starts
			if (m_current_cycle == m_start[m_current_environment + 1]){ // If the next environment should start next cycle
				// Initialise the next environment
				m_current_environment++;
				initialise(m_options, m_environment[m_current_environment]);
			}
		}

}

// Generic initialiser
void Composite::initialise(options_t &options, int environment){
	switch(environment){
	case 0:
		// Initialise coinflip1
		// Determine the probability of the coin landing on heads
		p = 1.0;
		if (options.count("coin-flip-p") > 0) {
			strExtract(options["coin-flip-p"], p);
		}
		assert(0.0 <= p);
		assert(p <= 1.0);

		std::cout << "Initialisation p = " << p <<std::endl;

		// Set up the initial observation
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = 0;

		break;
	case 1:
		// Initialise coinflip2
		// Determine the probability of the coin landing on heads
		p = 1.0;
		if (options.count("coin-flip-p2") > 0) {
			strExtract(options["coin-flip-p2"], p);
		}
		assert(0.0 <= p);
		assert(p <= 1.0);

		std::cout << "Initialisation p = " << p <<std::endl;

		// Set up the initial observation
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = 0;

		break;
	case 2:
		// Initialise Environment 2
		break;
	case 3:
		// Initialise Environment 3
		break;
	case 4:
		// Initialise Environment 4
		break;
	default:
		// Initialise coinflip1 by default
		// Determine the probability of the coin landing on heads
		p = 1.0;
		if (options.count("coin-flip-p") > 0) {
			strExtract(options["coin-flip-p"], p);
		}
		assert(0.0 <= p);
		assert(p <= 1.0);

		// Set up the initial observation
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = 0;

		break;
	}
}

void Composite::resolveAction(action_t action, int environment){
	switch(environment){
	case 0:
		// Resolve action for coinflip1
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = action == m_observation ? 1 : 0;

		break;
	case 1:
		// Resolve action for coinflip2
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = action == m_observation ? 1 : 0;
		break;
	case 2:
		// Resolve action for Environment 2
		break;
	case 3:
		// Resolve action for Environment 3
		break;
	case 4:
		// Resolve action for Environment 4
		break;
	default:
		// Resolve action for coinflip by default
		m_observation = rand01() < p ? 1 : 0;
		m_signed_reward = action == m_observation ? 1 : 0;
		break;
	}
}
