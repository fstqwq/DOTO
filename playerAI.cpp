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
point base, crys[2];

void init() {
	srand(time(NULL));

	ally = logic->faction;
	enemy = logic->faction ^ 1;
	base = map.birth_places[ally][0];

	for (int i = 0; i < 2; i++) crys[i] = map.crystal_places[i];

	double dis[2];
	for (int i = 0; i < 2; i++) {
		rush_to(base, map.bonus_places[i]);
		dis[i] = last_go_to_dis;
	}
	mybonus = dis[1] < dis[0];

}


Human getHuman(int x, int y) {return humans[y * facnum + x];}

point target[hunum], Move[hunum], Shoot[hunum], Meteor[hunum];
point mypos[hunum];
vector <point> history[hunum]; // enemy history
const maxHistory = 10;
int attack[hunum];
bool is_guard[hunum];

void adjust_movement() {
	for (int i = 0; i < hunum; i++) {
		for (int j = i + 1; j < hunum; j++) {
			if ((Move[i] - Move[j]).len() < fireball_radius) {
				point newtarget = mypos[i] + (Move[i] - mypos[i]).turn(-5);
				if (Legal(newtarget)) Move[i] = newtarget;
				else {
					newtarget = mypos[j] + (Move[j] - mypos[j]).turn(5);
					if (Legal(newtarget)) Move[j] = newtarget;
				}
			}
		}
	}
}


void frame_update() {
	for (int i = 0; i < hunum; i++) target[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
	for (int i = 0; i < hunum; i++) attack[i] = -1, is_guard[i] = 0;

	for (int i = 0; i < hunum; i++) {
		mypos[i] = (getHuman(enemy, i).position);
	}
	for (int i = 0; i < hunum; i++) {
		history[i].push_back(getHuman(enemy, i).position);	
		if (history[i].size() > maxHistory) history[i].pop_front();
	}

}

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

point forecast(int i, int step = 10) {
	Human &x = getHuman(enemy, i);
	point dir(0, 0);
	if (history[i].size() > 3) {
		dir = (x.position - history[i].front()).unit() * 0.5 + (x.position - history[i][history[i].size() - 2]).unit() * 0.5;
	}
	return history[i].back() + dir * step;
}

void solve() {
	logic = Logic::Instance();
	if (logic->frame == 1) init();	

	frame_update();

	target[0] = map.bonus_places[mybonus];

	if (crystal[enemy].belong != -1) {
		// Protect crystal belonger
		int belonger = crystal[enemy].belong;
		for (int i = 1; i < n; i++) {
			if (i == belonger) target[i] = crys[ally];
			else {
				target[i] = relative_pos(getHuman(ally, belonger).position, crys[enemy], explode_radius + 0.01);
				is_guard[i] = 1;
			}
		}
	}
	else {
		for (int i = 1; i < n; i++) {
			target[i] = crystal[enemy].position;
		}
	}

	for (int i = 0; i < 5; i++) {
		// TODO : Dodge
		move(i, rush_to(getHuman(ally, i), target[i]));
	}
	
	for (int i = 0; i < hunum; i++) find_enemy(i);

	for (int i = 0; i < hunum; i++) {
		point my = getHuman(ally, i), att = getHuman(enemy, attack[i]);
		shoot(i, forecast(attack[i]), int((att - my) / (fireball_velocity + human_velocity)));
		meteor(i, forecast(attack[i]), meteor_delay);
	}

	adjust_movement(); // Be seperate from others
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
