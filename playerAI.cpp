#include "playerAI.h"
#include "pathfinding.cpp"
#include <algorithm>
#include <ctime>

using namespace CONST;

namespace fstqwq {

using pii = std::pair<int, int>;

Logic *logic;

#define time_of_game 	logic->map.time_of_game
#define humans 			logic->humans
#define fireballs		logic->fireballs
#define meteors			logic->meteors
#define crystal			logic->crystal
#define map				logic->map
// birth_places[i][j]
// crystal_places[i]
// target_places[i]
// bonus_places[i]

int ally, enemy;
const int facnum = 2, hunum = 5; // assert(facnum == 2), assert(hunum == 5);
int mybonus;
point base;

void init() {
	srand(time(NULL));

	ally = logic->faction;
	enemy = logic->faction ^ 1;
	base = map.birth_places[ally][0];

	double dis[2];
	for (int i = 0; i < 2; i++) {
		rush_to(base, map.bonus_places[i]);
		dis[i] = last_go_to_dis;
	}
	mybonus = dis[1] < dis[0];

}


Human getHuman(int x, int y) {return humans[y * facnum + x];}

point target[hunum], Move[hunum], Shoot[hunum], Meteor[hunum];

void move(int i, const point &p) { // auto flash
	Move[i] = p;
	logic->move(i, p);

	if ((p - getHuman(ally, i).position) < CONST::human_velocity + eps) {
		logic->flash(i);
	}
	else {
		logic->unflash(i);
	}
}
void shoot(int i, const point &p) {
	logic->shoot(i, p);
	Shoot[i] = p;
}
void meteor(int i, const point &p) {
	logic->meteor(i, p);
	Meteor[i] = p;
}

void frame_init() {
	for (int i = 0; i < hunum; i++) target[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
}

void solve() {
	logic = Logic::Instance();
	if (logic->frame == 1) init();	
	frame_init();


	move(0, go_to(getHuman(ally, 0), map.bonus_places[mybonus]));

	for (int i = 1; i < 5; i++) {
		if (crystal[enemy].belong == i) {
			move(i, rush_to(getHuman(ally, i), base));
		}
		else {
			move(i, rush_to(getHuman(ally, i), crystal[enemy].position));
		}
	}
	
	for (int i = 0; i < hunum; i++) {
		int att = crystal[enemy].belong == -1 ? i : crystal[enemy].belong;
		shoot(i, getHuman(enemy, att).position);
		meteor(i, getHuman(enemy, att).position);
	}

}

#undef time_of_game
#undef humans
#undef fireballs
#undef meteors
#undef crystal
#undef map

}


void playerAI() {
	fstqwq::solve();	
}
