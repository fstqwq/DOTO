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
point base, enbase, crys[facnum], targ[facnum];

void init() {
	// srand in functions.h

	ally = logic->faction;
	enemy = logic->faction ^ 1;
	base = map.birth_places[ally][0];
	enbase = map.birth_places[enemy][0];

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
const int rand_times = 250;

void frame_before() {
	
	logic->debug("");


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
		if (mydead[i]) {
			mypos[i] = base;
		}
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
	if ((Move[i] - p).len() < 3) return;
	logic->shoot(i, p);
	Shoot[i] = p;
}

void meteor(int i, const point &p) {
	logic->meteor(i, p);
	Meteor[i] = p;
}


point forecast(int i, int step) {
	if (i == -1) return Illegal;
	point dir(0, 0);
	if (step < 5 || history[i].size() < maxHistory) return enpos[i];

	point ed(0, 0), st(0, 0);
	int sample = maxHistory / 4, dis = maxHistory - sample;
	for (int j = 0; j < sample; j++) {
		ed = ed + history[i][j];
		st = st + history[i][j + dis];
	}
	ed = ed * (1. / sample);
	st = st * (1. / sample);

	dir = (ed - st) / dis;
/*	dir = (history[i].front() - history[i].back()) / (maxHistory - 1);*/

	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	if (dir.len() < human_velocity / 5) dir = point(0, 0);
	
	point ans = enpos[i];
	for ( ; step-- >= 0; ) {
		if (Legal(ans + dir)) ans = ans + dir;
		else break;
	}
	return ans;
}

int find_enemy(point x) {
	int att = -1;
	double dis = 1e9;
	for (int i = 0; i < hunum; i++) if (!endead[i]){
		go_to(x, enpos[i]);
		double tmp = last_go_to_dis;
		if (i == enbelong) tmp *= 0.8;
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
	is_guard[p] = 1;
	point b = map.bonus_places[bonus_id];
	if ((mypos[p] - b).len() < bonus_radius + EP) {
		int tar = has_enemy(b, bonus_radius);
		dodge_ratio[p] = 10;
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
			else if (canfire[p] && (enholdfire[tar] > 4 || (getHuman(enemy, tar).fire_time > 4))){
				no_flash[p] = 1;
				point ans = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(4.8, 0).turn(Rand(0, 359));
					if (Legal(x) && (abs((ans - mypos[p]).len() - human_velocity) > abs((x - mypos[p]).len() - human_velocity) || !Legal(ans))) ans = x;
				}
				goal[p] = ans;
			}
			else {
				no_fire[p] = 1;
				point ans = enpos[tar];
				for (int t = 0; t < rand_times; t++) {
					point x = enpos[tar] + point(3.0, 0).turn(Rand(0, 359)) * (Rand(0, 100) / 100.);
					if (Legal(x) && (x - mypos[p]).len() < human_velocity + EP && (x - b).len() < (ans - b).len()) ans = x;
				}
				goal[p] = ans;
			}
			if (!Legal(goal[p])) goal[p] = b;
		}
		else {
			no_flash[p] = 1;
			goal[p] = b;
		}
	}
	else {
		goal[p] = b;
	}
}

void get_crystal(vector <int> squad) {
	// squad.size() == 3
	point cp = crystal[enemy].position;
	bool far = (cp - targ[ally]).len() * 2 > (cp - targ[enemy]).len();
	if (far) {
		for (auto i : squad) if (!mydead[i]) {
			if (i == belonger) {
				goal[i] = targ[ally];
			}
			else {
				goal[i] = cp;
				if (belonger != -1) for (int t = 0; t < rand_times; t++) {
					point x = cp + point(fireball_radius * 2, 0).turn(Rand(0, 359));
					if (Legal(x) && (abs((goal[i] - mypos[i]).len() - human_velocity) > abs((x - mypos[i]).len() - human_velocity) || !Legal(goal[i]))) {
						goal[i] = x;
					}
				}
			}
		}
	}
	else {
		int best = squad.front();
		for (auto i : squad) {
			if (i == belonger) {
				best = i;
				break;
			}
			else if ((mypos[i] - cp).len() + getHuman(ally, i).hp / 2 < (mypos[best] - cp).len() + getHuman(ally, best).hp / 2) {
				best = i;
			}
		}
		for (auto i : squad) {
			if (i == best) {
				if (i == belonger) {
					goal[i] = targ[ally];
				}
				else {
					goal[i] = cp;
				}
			}
			else {
				goal[i] = targ[enemy];
				point dir;
				for (int d = int((mypos[best] - targ[ally]).len() / 4 + 0.5);
					 d-- > 0 && Legal(dir = go_to(goal[i], cp, 4));
					 goal[i] = dir);
			}
		}

	}


/*
	if (belonger != -1) {
		// Protect crystal belonger
		int guarded = 0;
		for (auto i : squad) if (!mydead[i]) {
			if (i == belonger) {
				goal[i] = targ[ally];
			}
			else if ((guarded < 1 && ((mypos[belonger] - targ[ally]).len() * 2.33 > (mypos[belonger] - targ[enemy]).len()) && ((mypos[belonger] - targ[ally]).len() > (ff_enemy(mypos[belonger]) - mypos[belonger]).len()))) {
				dodge_ratio[i] = 9;
				goal[i] = relative_pos(mypos[belonger], ff_enemy(mypos[belonger]), explode_radius * 2);
				is_guard[i] = 1;
				guarded++;
			}
			else {
				dodge_ratio[i] = 15;
				goal[i] = crystal[ally].position;
				point dir;
				for (int d = 6; d-- > 0 && Legal(dir = go_to(goal[i], targ[enemy], 4)); goal[i] = dir);
			}
		}
	}
	else {
		for (auto i : squad) goal[i] = crystal[enemy].position;
	}
*/
}

vector <::Fireball> nowfir;
vector <::Meteor> nowmet;


double Score(int id, point x) {
	if (!Legal(x)) return 1e9;
	double ret = 0;

	// keepmoving
	ret -= (x - mypos[id]).len() * dodge_ratio[id] * .05;

	if (Legal(goal[id])) {
		go_to(x, goal[id]);
		ret += last_go_to_dis;
	}
	else {
		ret += (goal[id] - x).len();
	}

	// away from ally
	for (int i = 0; i < 5; i++) if (!mydead[i] && i != id) {
		double dis = (Move[i] - x).len();
		ret += pow(max(0., (fireball_radius * 2 - dis)), 2) * dodge_ratio[id] * .05;
	}

	// away from enemy
	if (!is_guard[id]) for (int i = 0; i < 5; i++) if (!endead[i]) {
		double dis = (enpos[i] - x).len();
		ret += pow(max(0., 16 - dis), 2) * .005; // danger zone
	}

	// away from wall
	ret += max(0., fireball_radius * 2 - (x - point(disw[int(x.x)][int(x.y)])).len()) * .1;
	
	// XXX : test if non-linear system works

	for (auto &v : nowfir) {
		double hurt = 0;
		point p = v.position, dir = point(cos(v.rotation), sin(v.rotation)) * double(fireball_velocity);
		double nowr = fireball_radius;
		int t = 0;
		while (t < 9 && Legal(p)) {
			p = p + dir;
			double dis = (p - x).len();
			if (dis < nowr + EP) {
				hurt = 25;
			}
			else if (dis < fireball_radius) {
				hurt = max(hurt, ((nowr + human_velocity > dis ? 6 : 2) + pow(fireball_radius - dis, 2)) * ((15 - t) / 10.));
			}
			else if (dis < fireball_radius + human_velocity * 3) {
				hurt = max(hurt, .5 * ((9 - t) / 9.) * (fireball_radius + human_velocity * 3 - dis));
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
			ret += (nowr + human_velocity > dis ? 6 : 2) + pow(15 - v.last_time, 2) * (explode_radius - dis);
		}
		else if (dis < explode_radius +  human_velocity * 3) {
			ret += .5 * ((15 - v.last_time) / 15.) * (fireball_radius + human_velocity * 3 - dis);
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
			if ((x.position - Move[i]).len() < 60) {
				nowfir.push_back(x);
			}
		}
		for (auto &x : meteors) if (x.from_number % facnum != ally) {
			if ((x.position - Move[i]).len() < 60 && x.last_time < 15) {
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
			for (int d = 15; d < 360; d += 15) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}
		else {
			dir = point(human_velocity, 0);
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(10, 100) / 100.);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			for (int d = 0; d < 360; d += 10) {
				point p = mypos[i] + dir.turn(d);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}
		if (!no_flash[i] && canflash[i]) {
			dir = dir.unit() * flash_distance;
			for (int t = 0; t < rand_times; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(40, 100) / 100.);
				double tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}
		logic->debugAppend(to_string(i) + " : " + to_string(sc) + "\n");
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

	logic->debugAppend(to_string((clock() - st) / double(CLOCKS_PER_SEC)));
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
