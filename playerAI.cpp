#include "playerAI.h"
#include "pathfinding.cpp"
#include <algorithm>
#include <bits/stdc++.h>
using namespace CONST;

namespace fstqwq {

using namespace std;
using pii = std::pair<int, int>;

Logic *logic;

#define humans 			logic->humans
#define fireballs		logic->fireballs
#define meteors			logic->meteors
#define crystal			logic->crystal
#define map				logic->map
#define time_of_game 	map.time_of_game
// birth_places[i][j]
// crystal_places[i]
// target_places[i]
// bonus_places[i]

int ally, enemy;
const int facnum = 2, hunum = 5; // assert(facnum == 2), assert(hunum == 5);
const int rand_times = 30;
int mybonus;
point base, crys[facnum], targ[facnum];

void init() {
	// srand in functions.h

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
	point dir(0, 0);
	if (step < 4 || history[i].size() < maxHistory) return enpos[i];

	dir = (enpos[i] - history[i].back()) / maxHistory;

	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	if (dir.len() < human_velocity / 3) dir = point(0, 0);
	
	point ans = enpos[i];
	for ( ; step-- >= 0; ) {
		if (Legal(ans + dir)) ans = ans + dir;
		else break;
		dir = dir * 0.6;
	}
	return ans;
}

int find_enemy(point x) {
	int att = 0;
	double dis = 1e9;
	for (int i = 0; i < hunum; i++) if (!endead[i]){
		double tmp = g[col[int(x.x)][int(x.y)]][col[int(enpos[i].x)][int(enpos[i].y)]];
		if (tmp < dis) att = i, dis = tmp;
	}
	return att;
}

point ff_enemy(point x) {
	int att = -1;
	double dis = 1e9;
	for (int j = 0; j < hunum; j++) if (!endead[j]){
		double tmp = (enpos[j] - x).len();
		if (tmp < dis) att = j, dis = tmp;
	}
	return att == -1 ? point(1e9, 1e9) : enpos[att];
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
		if (tar != 1) {
			if (canflash[p] && canfire[p]) {
				point ans = bp;
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(splash_radius + fireball_velocity, 0).turn(Rand(0, 359)) * (Rand(90, 110) / 100.);
					if (Legal(x)) ans = x;
				}
				goal[p] = ans;
			}
			goal[p] = bp; 
		}
		else {
			goal[p] = bp;
		}
	}
	else {
		goal[p] = bp;
	}
}

void get_crystal(vector <int> squad) {
	//int cnt = 0, tot = (int)squad.size();
	//for (auto i : squad) cnt += !mydead[i];

	if (belonger != -1) {
		// Protect crystal belonger
		for (auto i : squad) {
			if (i == belonger) goal[i] = targ[ally];
			else {
				goal[i] = relative_pos(mypos[belonger], enpos[find_enemy(mypos[belonger])].turn((i - 2 - (i > belonger)) * 20 - 10), explode_radius * 2);
				is_guard[i] = 1;
			}
		}
	}
	else {
		double shortest = 1e9;
		for (auto i : squad) if (!mydead[i]) {
			shortest = min(shortest, (mypos[i] - crystal[enemy].position).len() + (crystal[enemy].position - targ[ally]).len());
		}
		if (0 && (time_of_game - logic->frame + 1) * .6 >= shortest - target_radius) {
			for (auto i : squad) {
				goal[i] = crystal[ally].position + point(0, 5).turn((i % 3) * 120);
				while (!Legal(Move[i])) {
					goal[i] = crystal[ally].position + point(0, 5).turn((i % 3) * 120) * (Rand(50, 100) / 100.);
				}
			}
		}
		else {
			for (auto i : squad) goal[i] = crystal[enemy].position;

			// Union
			for (auto i : squad) if (getHuman(ally, i).hp >= 50) {
				for (auto j : squad) if (getHuman(ally, j).hp >= 50) {
					double dis = (mypos[i] - mypos[j]).len();
					if (dis > 50) {
						int far = (mypos[i] - base).len() > (mypos[j] - base).len() ? i : j, near = i + j - far;
						if (has_enemy(mypos[far], (mypos[far] - crystal[enemy].position).len()) != -1) {
							goal[far] = mypos[near];
						}
					}
				}
			}
		}
	}
}

vector <::Fireball> nowfir;
vector <::Meteor> nowmet;

double Score(int id, point x) {
	if (!Legal(x)) return 1e9;
	double ret = 0;
	go_to(x, goal[id]);
	ret += last_go_to_dis;
	for (int i = 0; i < 5; i++) if (!mydead[i] && i != id) {
		double dis = (Move[i] - x).len();
		ret += max(0., fireball_radius * 2 - dis) * 0.1;
	}
	
	for (auto &v : nowfir) {
		int hurt = 0;
		point p = v.position;
		double nowr = fireball_radius;
		while (nowr > 0 && Legal(p)) {
			p = p + point(cos(v.rotation), sin(v.rotation)) * double(fireball_velocity);
			if ((p - x).len() < nowr + EP) {
				hurt += 16;
			}
			else if ((p - x).len() < nowr + human_velocity * 2) {
				// linear in (r, r + 2 v)
				hurt += 8 * (nowr + human_velocity * 2 - (p - x).len()) / (human_velocity * 2);
			}
			nowr -= human_velocity;
		}
		if (hurt) ret += hurt;
	}
	
	for (auto &v : nowmet) {
		bool hurt = 0;
		point p = v.position;
		double nowr = explode_radius - max(0, v.last_time - 1) * human_velocity * 0.95;
		if ((p - x).len() < nowr + EP) ret += 999;
		else if ((p - x).len() < nowr + human_velocity + EP) ret += 5;
	}
	return ret;
}

void adjust_movement() {
	for (int i = 0; i < 5; i++) if (!mydead[i]) {
		if (i == belonger) {
			Move[i] = go_to(mypos[i], goal[i]);
		}
		else {
			Move[i] = rush_to(getHuman(ally, i), goal[i]);
		}
	}

	// XXX : Dodge
	// Implement : Rand, and score every position
	// Target Dis + 0.1 * (Fireball_radius - Friend pos) + 5 * Fireball + 999 * meteor
	for (int i = 0; i < hunum; i++) if (!mydead[i]) {
		nowfir.clear();
		nowmet.clear();
		for (auto &x : fireballs) if (x.from_number % facnum != ally) {
			if ((x.position - Move[i]).len() < 50) {
				nowfir.push_back(x);
			}
		}
		for (auto &x : meteors) if (x.from_number % facnum != ally) {
			if ((x.position - Move[i]).len() < 50) {
				nowmet.push_back(x);
			}
		}

		double sc = Score(i, Move[i]);
		point dir = Move[i] - mypos[i];
		if (dir.len() > .3) { 
			// Change direction a little, but not change length
			for (int d = -20; d <= 20; d += 4) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			dir = dir.unit() * human_velocity;
			// Change direction a little, change length a little 
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(-60, 60)) * (Rand(80, 100) / 100.);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			// Change direction 
			for (int d = 30; d < 360; d += 30) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}
		else {
			dir = point(human_velocity, 0);
			for (int d = 0; d < 360; d += 10) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
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
