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
bool is_guard[hunum], no_flash[hunum], no_fire[hunum];
int belonger, enbelong;


double dodge_ratio[hunum];

vector <point> history[hunum]; // enemy history
int enholdfire[hunum];
const int maxHistory = 20;
const int rand_times = 100;

void frame_before() {
	for (int i = 0; i < hunum; i++) goal[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
	for (int i = 0; i < hunum; i++) attack[i] = -1, is_guard[i] = 0;
	for (int i = 0; i < hunum; i++) no_flash[i] = 0, no_fire[i] = 0;
	for (int i = 0; i < hunum; i++) dodge_ratio[i] = 1;

	belonger = crystal[enemy].belong;
	enbelong = crystal[ally].belong;
	if (belonger != -1) belonger /= facnum;
	if (enbelong != -1) enbelong /= facnum;

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
		if (x.fire_time || endead[i]) enholdfire[i] = 0;
		else enholdfire[i]++;
	}
	for (int i = 0; i < hunum; i++) {
		if (endead[i]) history[i].clear();
		else {
			history[i].insert(history[i].begin(), enpos[i]);	
			if (history[i].size() > maxHistory) history[i].pop_back();
		}
	}
}

void frame_after() {

}

void move(int i, point p) { // auto flash
	double dis = (mypos[i] - p).len();
	
	if (dis > flash_distance + EP) {
		point newp = mypos[i] + (p - mypos[i]) / dis * flash_distance;
		if (Legal(newp)) p = newp, dis = flash_distance;
		else return;
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
	if (no_fire[i]) return;
	if ((Move[i] - p).len() < 3.2) return;
	logic->shoot(i, p);
	Shoot[i] = p;
}

void meteor(int i, const point &p) {
	logic->meteor(i, p);
	Meteor[i] = p;
}


//TODO : avrange

point forecast(int i, int step) {
	point dir(0, 0);
	if (step < 4 || history[i].size() < maxHistory) return enpos[i];

	point ed(0, 0), st(0, 0);
	for (int j = 0; j < maxHistory / 4; j++) {
		ed = ed + history[i][j];
		st = st + history[i][j + maxHistory / 2];
	}
	ed = ed / (maxHistory / 2.);
	st = st / (maxHistory / 2.);

	dir = (ed - st) / (maxHistory / 2);

/*	dir = (history[i].front() - history[i].back()) / (maxHistory - 1);*/
	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	if (dir.len() < human_velocity / 3) dir = point(0, 0);
	
	point ans = enpos[i];
	for ( ; step-- >= 0; ) {
		if (Legal(ans + dir)) ans = ans + dir;
		else break;
	}
	return ans;
}

int find_enemy(point x) {
	int att = 0;
	double dis = 1e9;
	for (int i = 0; i < hunum; i++) if (!endead[i]){
		double tmp = g[col[int(x.x)][int(x.y)]][col[int(enpos[i].x)][int(enpos[i].y)]];
		if (i == enbelong) tmp /= 2;
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

void get_bonus(int bonus_id, int p) {
	dodge_ratio[p] = 5;
	point b = map.bonus_places[bonus_id];
	if ((mypos[p] - b).len() < bonus_radius + EP) {
		int tar = has_enemy(b, bonus_radius);
		if (tar != -1) {
			if (canflash[p] && canfire[p]) {
				point ans = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
				// flash
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
					if (Legal(x) && ((ans - mypos[p]).len() < (x - mypos[p]).len() || !Legal(ans))) ans = x;
				}
				goal[p] = ans;
			}
			else if (canfire[p] && (enholdfire[tar] || (getHuman(enemy, tar).fire_time < 8 && getHuman(enemy, tar).fire_time > 3))){
				no_flash[p] = 1;
				point ans = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
					if (Legal(x) && (abs((ans - mypos[p]).len() - 0.59) > abs((x - mypos[p]).len() - 0.59) || !Legal(ans))) ans = x;
				}
				goal[p] = ans;
			}
			else {
				point ans = enpos[tar] + point(3.1, 0).turn(Rand(0, 359));
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(3.1, 0).turn(Rand(0, 359));
					if (Legal(x) && (abs((ans - mypos[p]).len() - 0.59) > abs((x - mypos[p]).len() - 0.59) || !Legal(ans))) ans = x;
				}
				goal[p] = ans;
			}
			if (!Legal(goal[p])) goal[p] = b;
		}
		else {
			dodge_ratio[p] = 10; 
			no_flash[p] = 1;
			goal[p] = b;
		}
	}
	else {
		goal[p] = b;
	}
}

void get_crystal(vector <int> squad) {
	//int cnt = 0, tot = (int)squad.size();
	//for (auto i : squad) cnt += !mydead[i];

	if (belonger != -1) {
		// Protect crystal belonger
		int guarded = 0;
		for (auto i : squad) if (!mydead[i]) {
			if (i == belonger) {
				dodge_ratio[i] = 1;
				goal[i] = targ[ally];
			}
			else if (guarded < 1) {
				goal[i] = relative_pos(mypos[belonger], enpos[find_enemy(mypos[belonger])], explode_radius * 2);
				is_guard[i] = 1;
				guarded++;
			}
			else {
				dodge_ratio[i] = 10;
				goal[i] = crystal[ally].position;
				point dir = (targ[enemy] - goal[i]).unit();
				for (int d = 60; d-- && Legal(goal[i] + dir); d--) goal[i] = goal[i] + dir;
			}
		}
	}
	else {
		// TODO : Counter divide
		double shortest = 1e9;
		for (auto i : squad) if (!mydead[i]) {
			shortest = min(shortest, (mypos[i] - crystal[enemy].position).len() + (crystal[enemy].position - targ[ally]).len());
		}
		if (0 && (time_of_game - logic->frame + 1) * .6 >= shortest - target_radius) {
			for (auto i : squad) goal[i] = crystal[ally].position;
		}
		else {
			for (auto i : squad) goal[i] = crystal[enemy].position;
/*
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
			}*/
		}
	}
}

vector <::Fireball> nowfir;
vector <::Meteor> nowmet;


// TODO : dodge_ratio
// XXX : Dodge
// Implement : Rand, and score every position
// Target Dis + 0.1 * (Fireball_radius - Friend pos) + 5 * Fireball + 999 * meteor
/*
double Score(int id, point x) {
	if (!Legal(x)) return 1e9;
	double ret = 0;

	go_to(x, goal[id]);
	ret += last_go_to_dis;

	for (int i = 0; i < 5; i++) if (!mydead[i] && i != id) {
		double dis = (Move[i] - x).len();
		ret += max(0., fireball_radius * 2 - dis) * .1 * dodge_ratio[id];
	}

	ret += max(0., fireball_radius - disw[int(x.x)][int(x.y)]) * .1 * dodge_ratio[id];
	
	for (auto &v : nowfir) {
		double hurt = 0;
		point p = v.position, dir = point(cos(v.rotation), sin(v.rotation)) * double(fireball_velocity);
		double nowr = fireball_radius;
		int t = 0;
		while (Legal(p)) {
			p = p + dir;
			double dis = (p - x).len();
			if (dis < nowr + EP) {
				hurt = 24;
			}
			else if (dis < fireball_radius + human_velocity * 2) {
				hurt = max(hurt, ((dis < fireball_radius ? 8 : 0) + (fireball_radius - dis)) * exp((10 - t) / 6.));
			}
			nowr -= human_velocity * 0.99;
			t++;
		}
		ret += hurt * dodge_ratio[id];
	}
	
	for (auto &v : nowmet) {
		point p = v.position;
		int t = v.last_time - 1; 
		double nowr = explode_radius - t * human_velocity * 0.99;
		double dis = (p - x).len();
		if (dis < nowr + EP) {
			ret += 1e9;
		}
		else if (dis < explode_radius + human_velocity) {
			ret += 5 * (10 - t) + (explode_radius - dis) * exp((10 - t) / 3.);
		}
	}
	return ret;
}
*/
double Score(int id, point x) {
	if (!Legal(x)) return 1e9;
	double ret = 0;
	go_to(x, goal[id]);
	
	ret += last_go_to_dis;

	// away from ally
	for (int i = 0; i < 5; i++) if (!mydead[i] && i != id) {
		double dis = (Move[i] - x).len();
		ret += max(0., fireball_radius * 2 - dis) * .1 * dodge_ratio[id];
	}

	// away from wall
	ret += max(0., fireball_radius * 2 - (x - point(disw[int(x.x)][int(x.y)])).len()) * .1 * dodge_ratio[id];
	
	for (auto &v : nowfir) {
		double hurt = 0;
		point p = v.position, dir = point(cos(v.rotation), sin(v.rotation)) * double(fireball_velocity);
		double nowr = fireball_radius;
		int t = 0;
		while (t < 9 && Legal(p)) {
			p = p + dir;
			double dis = (p - x).len();
			if (dis < nowr + EP) {
				hurt = 24;
			}
			else if (dis < fireball_radius + human_velocity * 2) {
				hurt = max(hurt, (dis < fireball_radius ? 8 : 0) * ((8 - t) / 5.) + 3 * (fireball_radius + human_velocity - dis));
			}
			nowr -= human_velocity;
			t++;
		}
		ret += hurt * dodge_ratio[id];
	}
	
	for (auto &v : nowmet) {
		point p = v.position;
		double nowr = explode_radius - (v.last_time - 1) * human_velocity;
		double dis = (p - x).len();
		if (dis < nowr + EP) {
			ret += 1e9;
		}
		else if (dis < explode_radius + EP) {
			ret += (10 - v.last_time) + (explode_radius - dis) * exp(10 - v.last_time);
		}
		else if (dis < explode_radius + human_velocity) {
			ret += (10 - v.last_time) * (explode_radius + human_velocity - dis);
		}
	}
	return ret;
}

void adjust_movement() {
	for (int i = 0; i < 5; i++) if (!mydead[i]) {
		if (!canflash[i] || no_flash[i]) {
			Move[i] = go_to(mypos[i], goal[i]);
		}
		else {
			Move[i] = rush_to(mypos[i], goal[i]);
		}
	}

	for (int i = 0; i < hunum; i++) if (!mydead[i]) {
		nowfir.clear();
		nowmet.clear();
		for (auto &x : fireballs) if (x.from_number % facnum != ally) {
			if ((x.position - Move[i]).len() < 50) {
				nowfir.push_back(x);
			}
		}
		for (auto &x : meteors) if (x.from_number % facnum != ally) {
			if ((x.position - Move[i]).len() < 50 && x.last_time < 10) {
				nowmet.push_back(x);
			}
		}

		double sc = Score(i, Move[i]);
		point dir = Move[i] - mypos[i];
		if (dir.len() > .1) { 
			// Change direction a little, but not change length
			for (int d = -30; d <= 30; d += 4) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			dir = dir.unit() * human_velocity;
			// Change everything 
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(0, 100) / 100.);
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
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(0, 100) / 100.);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			for (int d = 0; d < 360; d += 15) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}
		if (!no_flash[i] && canflash[i]) {
			dir = dir.unit() * flash_distance;
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(20, 100) / 100.);
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
	time_t st = clock(); 

	logic = Logic::Instance();
	if (logic->frame == 1) init();	

	frame_before();

	get_bonus(0, 0);
	get_bonus(1, 1);
	get_crystal({2, 3, 4});
	adjust_movement();
	damage();	

	frame_after();

	logic->debug(to_string((clock() - st) / double(CLOCKS_PER_SEC)));
	logic->debugAppend("\n" + to_string(getHuman(ally, 0).fire_time));
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
