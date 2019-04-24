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
int facnum, hunum; // assert(facnum == 2), assert(hunum == 5);
int mybonus;
point base;

void init() {
	srand(time(NULL));

	ally = logic->faction;
	enemy = logic->faction ^ 1;
	facnum = map.faction_number;
	hunum = map.human_number;
	base = map.birth_places[ally][0];

	double dis[2];
	for (int i = 0; i < 2; i++) {
		go_to(base, map.bonus_places[i]);
		dis[i] = last_go_to_dis;
	}
	mybonus = dis[1] < dis[0];

}


Human getHuman(int x, int y) {return humans[y * facnum + x];}


void solve() {
	logic = Logic::Instance();
	if (logic->frame == 1) init();	
	
	logic->move(0, go_to(getHuman(ally, 0).position, map.bonus_places[mybonus]));

	for (int i = 1; i < 5; i++) {
		if (crystal[enemy].belong == i) {
			logic->move(i, go_to(getHuman(ally, i).position, base));
		}
		else {
			logic->move(i, go_to(getHuman(ally, i).position, crystal[enemy].position));
		}
	}
	
	for (int i = 0; i < hunum; i++) {
		int att = crystal[enemy].belong == -1 ? i : crystal[enemy].belong;
		logic->shoot(i, getHuman(enemy, att).position);
		logic->meteor(i, getHuman(enemy, att).position);
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
