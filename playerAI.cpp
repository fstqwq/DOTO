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
point base, crys[2], targ[2];

void init() {
	srand(time(NULL));

	ally = logic->faction;
	enemy = logic->faction ^ 1;
	base = map.birth_places[ally][0];

	for (int i = 0; i < 2; i++) crys[i] = map.crystal_places[i];
	for (int i = 0; i < 2; i++) targ[i] = map.target_places[i];

	double dis[2];
	for (int i = 0; i < 2; i++) {
		go_to(base, map.bonus_places[i]);
		dis[i] = last_go_to_dis;
	}
	mybonus = dis[1] < dis[0];

}


Human& getHuman(int x, int y) {return humans[y * facnum + x];}

point goal[hunum], Move[hunum], Shoot[hunum], Meteor[hunum], mypos[hunum], enpos[hunum];
int attack[hunum];
bool is_guard[hunum];
vector <point> history[hunum]; // enemy history
const int maxHistory = 15;


void frame_update() {
	for (int i = 0; i < hunum; i++) goal[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
	for (int i = 0; i < hunum; i++) attack[i] = -1, is_guard[i] = 0;

	for (int i = 0; i < hunum; i++) {
		mypos[i] = (getHuman(ally, i).position);
	}
	for (int i = 0; i < hunum; i++) {
		enpos[i] = (getHuman(enemy, i).position);
	}
	for (int i = 0; i < hunum; i++) {
		if (getHuman(enemy, i).death_time) history[i].clear();
		history[i].push_back(getHuman(enemy, i).position);	
		if (history[i].size() > maxHistory) history[i].erase(history[i].begin());
	}

}

void move(int i, const point &p) { // auto flash
	Move[i] = p;
	logic->move(i, p);

	if ((p - getHuman(ally, i).position).len() > CONST::human_velocity + eps) {
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

void adjust_movement() {
	for (int i = 0; i < hunum; i++) {
		for (int j = i + 1; j < hunum; j++) {
			if ((Move[i] - Move[j]).len() <= fireball_radius + 0.01) {
				point newgoal = mypos[i] + (Move[i] - mypos[i]).turn(Rand(-4 + i, 0 + i));
				if (Legal(newgoal)) Move[i] = newgoal;
				else {
					newgoal = mypos[j] + (Move[j] - mypos[j]).turn(Rand(-4 + j, 0 + j));
					if (Legal(newgoal)) Move[j] = newgoal;
				}
			}
		}
	}
	for (int i = 0; i < hunum; i++) move(i, Move[i]);
}


point forecast(int i, int step = 10) {
	point dir(0, 0);
	if (history[i].size() <= 3) return enpos[i];
	dir = (enpos[i] - history[i].front()) / (history[i].size() - 1);
	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	return enpos[i] + dir * step;
}

int find_enemy(point x) {
	int att = 0;
	double dis = (enpos[0] - x).len();
	for (int j = 1; j < hunum; j++) {
		double tmp = (enpos[j] - x).len();
		if (tmp < dis) att = j, dis = tmp;
	}
	return att;
}

void solve() {
	logic = Logic::Instance();
	if (logic->frame == 1) init();	

	frame_update();
	
	if ((goal[0] - map.bonus_places[mybonus]).len() < bonus_radius) {
		goal[0] = map.bonus_places[mybonus] + (mypos[0] - map.bonus_places[mybonus]).turn(10);
	}
	else {
		goal[0] = map.bonus_places[mybonus] + point(Rand(0, 3), Rand(-3, -1));
	}

	int belonger = crystal[enemy].belong;
	if (crystal[enemy].belong != -1) {
		// Protect crystal belonger
		belonger /= facnum;
		for (int i = 1; i < hunum; i++) {
			if (i == belonger) goal[i] = targ[ally];
			else {
				goal[i] = relative_pos(mypos[belonger], enpos[find_enemy(mypos[belonger])], explode_radius + 0.01);
				is_guard[i] = 1;
			}
		}
	}
	else {
		for (int i = 1; i < hunum; i++) {
			goal[i] = crystal[enemy].position;
		}
	}

	for (int i = 0; i < 5; i++) {
		// TODO : Dodge
		if (i == belonger) {
			move(i, go_to(mypos[i], goal[i]));
		}
		else {
			move(i, rush_to(getHuman(ally, i), goal[i]));
		}
	}
	
	for (int i = 0; i < hunum; i++) attack[i] = find_enemy(mypos[i]);

	for (int i = 0; i < hunum; i++) {
		point my = mypos[i], att = enpos[attack[i]];
		shoot(i, forecast(attack[i], int((att - my).len() / (fireball_velocity + human_velocity))));
		meteor(i, forecast(attack[i], meteor_delay));
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
