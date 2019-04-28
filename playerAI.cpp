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
point base, crys[facnum], targ[facnum];

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

point goal[hunum], Move[hunum], Shoot[hunum], Meteor[hunum];
point mypos[hunum], enpos[hunum];
bool mydead[hunum], endead[hunum];
bool canflash[hunum], canfire[hunum];

int attack[hunum];
bool is_guard[hunum];
int belonger;

vector <point> history[hunum]; // enemy history
const int maxHistory = 20;

void frame_before() {
	for (int i = 0; i < hunum; i++) goal[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
	for (int i = 0; i < hunum; i++) attack[i] = -1, is_guard[i] = 0;

	belonger = crystal[enemy].belong;
	if (belonger != -1) belonger /= facnum;

	for (int i = 0; i < hunum; i++) {
		const Human& x = getHuman(ally, i);
		mypos[i] = x;
		mydead[i] = !x.hp;
		canflash[i] = !x.flash_time && x.flash_num && i != belonger;
		canfire[i] = !x.fire_time;
	}
	for (int i = 0; i < hunum; i++) {
		const Human& x = getHuman(enemy, i);
		enpos[i] = x.position;
		endead[i] = !x.hp;
	}
	for (int i = 0; i < hunum; i++) {
		if (endead[i]) history[i].clear();
	}
}

void frame_after() {
	for (int i = 0; i < hunum; i++) {
		history[i].insert(history[i].begin(), enpos[i]);	
		if (history[i].size() > maxHistory) history[i].pop_back();
	}
}

void move(int i, point p) { // auto flash
	double dis = (mypos[i] - p).len();
	
	if (dis > flash_distance + EP) {
		point newp = mypos[i] + (p - mypos[i]) / dis * flash_distance;
		if (Legal(newp)) p = newp, dis = flash_distance;
	}
	
	if (dis > human_velocity + EP && dis < human_velocity * 2) {
		point newp = mypos[i] + (p - mypos[i]) / dis * human_velocity;
		if (Legal(newp)) p = newp, dis = human_velocity;
	}

	Move[i] = p;
	logic->move(i, p);

	if (dis > CONST::human_velocity + EP) {
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

point forecast(int i, int step) {
	return enpos[i];
	point dir(0, 0);
	if (step < 4 || history[i].size() < maxHistory) return enpos[i];

	dir = (enpos[i] - history[i].back()) / maxHistory;

	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	if (dir.len() < human_velocity / 3) dir = point(0, 0);
	point ans = enpos[i];
	while (step-- > 0) {
		if (Legal(ans + dir)) ans = ans + dir;
	}
	return ans;
}

int find_enemy(point x) {
	int att = 0;
	double dis = 1e10;
	for (int j = 0; j < hunum; j++) if (!endead[j]){
		double tmp = (enpos[j] - x).len();
		if (tmp < dis) att = j, dis = tmp;
	}
	return att;
}

int has_enemy(point x, double r) {
	for (int j = 0; j < hunum; j++) if (!endead[j]) {
		double tmp = (enpos[j] - x).len();
		if (tmp < r + EP) return j;
	}
	return -1;
}

void get_bonus(int id, int p) {
	point bp = map.bonus_places[id];
	if ((mypos[p] - bp).len() < bonus_radius + EP) {
		int tar = has_enemy(bp, bonus_radius);
		if (~tar) {
			if (canflash[p] && canfire[p]) {
				point dir(splash_radius + 0.1, 0);
				while (!Legal(enpos[tar] + dir)) dir = dir.turn(Rand(0, 359));
				goal[p] = enpos[tar] + dir;
			}
			else {
				point dir(splash_radius + 0.1, 0), ans;
				for (int i = 0; i < 50; i++) {
					point tmp; 
					while (!Legal(tmp = enpos[tar] + dir)) dir = dir.turn(Rand(0, 359));
					if (i == 0 || (ans - mypos[p]).len() > (tmp - mypos[p]).len()) ans = tmp;
				}
				goal[p] = ans;
			}
		}
		else {
			goal[p] = bp + ((mypos[p] == bp ? point(0, 1) : mypos[p] - bp).unit() * bonus_radius * (Rand(80, 100) / 100.)).turn(human_velocity / bonus_radius);
		}
	}
	else {
		goal[p] = bp;
	}
}

void get_crystal(vector <int> squad) {
	if (belonger != -1) {
		// Protect crystal belonger
		for (auto i : squad) {
			if (i == belonger) goal[i] = targ[ally];
			else {
				goal[i] = relative_pos(mypos[belonger], enpos[find_enemy(mypos[belonger])].turn(i * 20 - 50), explode_radius * 2);
				is_guard[i] = 1;
			}
		}
	}
	else {
		for (auto i : squad) {
			goal[i] = crystal[enemy].position;
		}
	}
}

void adjust_movement() {
	for (int i = 0; i < 5; i++) {
		// TODO : Dodge
		if (i == belonger) {
			move(i, go_to(mypos[i], goal[i]));
		}
		else {
			move(i, rush_to(getHuman(ally, i), goal[i]));
		}
	}

	for (int i = 0; i < hunum; i++) {
		for (int j = 0; j < hunum; j++) if (i != j) {
			double dis = (Move[i] - Move[j]).len();
			if (dis <= fireball_radius * 2) {
				point newgoal = mypos[i] + (Move[i] - mypos[i]).turn(Rand(-10, 10));
				while ((newgoal - Move[j]).len() < dis) newgoal = mypos[i] + (Move[i] - mypos[i]).turn(Rand(-10, 10));
				if (Legal(newgoal)) Move[i] = newgoal;
			}
		}
	}
	for (int i = 0; i < hunum; i++) move(i, Move[i]);
}

void damage() {
	for (int i = 0; i < hunum; i++) attack[i] = find_enemy(Move[i]);

	for (int i = 0; i < hunum; i++) {
		point my = Move[i], att = enpos[attack[i]];
		shoot(i, forecast(attack[i], 1 + int((att - my).len() / (fireball_velocity + human_velocity))));
		meteor(i, forecast(attack[i], meteor_delay));
	}
}

void solve() {
	logic = Logic::Instance();
	if (logic->frame == 1) init();	

	frame_before();

	get_bonus(mybonus, 0);
	get_bonus(mybonus ^ 1, 1);
	get_crystal({2, 3, 4});
	adjust_movement();
	damage();	

	frame_after();
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
