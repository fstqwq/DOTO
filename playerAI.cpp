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
bool is_guard[hunum], no_flash[hunum], nofire[hunum];
int belonger, enbelong;

double goalr[hunum];
double dodge_ratio[hunum];

vector <point> history[hunum]; // enemy history
int enholdfire[hunum];
const int maxHistory = 18;
const int rand_times = 266;

int frame = 0;
void frame_before() {
	
	if (frame != logic->frame - 1) {
		logic->debug("Skipped frame : " + to_string(frame) + " " + to_string(logic->frame) + "\n");
	}
	frame = logic->frame;

	logic->debug("");

	for (int i = 0; i < 2; i++) crys[i] = map.crystal_places[i];
	for (int i = 0; i < hunum; i++) goal[i] = Move[i] = Shoot[i] = Meteor[i] = Illegal;
	for (int i = 0; i < hunum; i++) goalr[i] = 0; 
	for (int i = 0; i < hunum; i++) attack[i] = -1, is_guard[i] = 0;
	for (int i = 0; i < hunum; i++) no_flash[i] = 0, nofire[i] = 0;
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
	
	if (dis > human_velocity + EP && dis < human_velocity + 0.1) {
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


// debug
//
int maxadj = 0;

void shoot(int i, const point &p) {
	if (nofire[i] || !canfire[i]) return;
	
	point dir = (p - Move[i]);
	if (dir.len() < 3) return;

	dir = dir.unit();

	int best = 0;
	double dis = 1e9;
	for (int d = -0; d <= 0; d += 1) {
		point dd = dir.turn(d);
		double tmp = 1e9;
		point x = Move[i] + dd * 3; 
		if (!Legal(x)) continue;
		for ( ; Legal(x); x = x + dd) {
			tmp = min(tmp, (x - p).len());
		}
		if (tmp < dis) {
			dis = tmp;
			best = d;
		}
	}
	if (dis > 4) {
		for (int d = -100; d < -40; d += 4) {
			point dd = dir.turn(d);
			double tmp = 1e9;
			point x = Move[i] + dd * 3; 
			if (!Legal(x)) continue;
			for ( ; Legal(x); x = x + dd) {
				tmp = min(tmp, (x - p).len());
			}
			if (tmp < dis) {
				dis = tmp;
				best = d;
			}
		}
		for (int d = -40; d <= 40; d += 1) {
			point dd = dir.turn(d);
			double tmp = 1e9;
			point x = Move[i] + dd * 3; 
			if (!Legal(x)) continue;
			for ( ; Legal(x); x = x + dd) {
				tmp = min(tmp, (x - p).len());
			}
			if (tmp < dis) {
				dis = tmp;
				best = d;
			}
		}
		for (int d = 44; d <= 100; d += 4) {
			point dd = dir.turn(d);
			double tmp = 1e9;
			point x = Move[i] + dd * 3; 
			if (!Legal(x)) continue;
			for ( ; Legal(x); x = x + dd) {
				tmp = min(tmp, (x - p).len());
			}
			if (tmp < dis) {
				dis = tmp;
				best = d;
			}
		}
	}
	Shoot[i] = Move[i] + dir.turn(best) * 3;
	logic->shoot(i, Shoot[i]);
	maxadj = max(maxadj, abs(best));
/*	if (best != 0) {
		logic->debugAppend("adjusted : " + to_string(i) + " : " + to_string(dir.turn(best).x) + ", " + to_string(dir.turn(best).y) + " : "  + to_string(dir.x) + ", " + to_string(dir.y) + "\n");
	}*/
}

void meteor(int i, const point &p) {
	if ((p - Move[i]).len() > meteor_distance) {
		return;
	}
	if (is_guard[i] && (frame - 250) % (8 * frames_per_second) !=  (8 * frames_per_second) - meteor_delay) {
		return;
	}
	logic->meteor(i, p);
	Meteor[i] = p;
}


point forecast(int i, int step, point aux = Illegal) {
	if (i == -1) return Illegal;
	point dir(0, 0);
	if (history[i].size() < maxHistory) {
		return enpos[i];
	}

	point ed(0, 0), st(0, 0);
	int sample = 3, dis = maxHistory - sample;
	for (int j = 0; j < sample; j++) {
		ed = ed + history[i][j];
		st = st + history[i][j + dis];
	}
	ed = ed * (1. / sample);
	st = st * (1. / sample);

	dir = (ed - st) / dis;
/*	dir = (history[i].front() - history[i].back()) / (maxHistory - 1);*/

	if (dir.len() > human_velocity) dir = dir.unit() * human_velocity;
	point ans = enpos[i];
	bool okk = 0;
	if (dir.len() < human_velocity / 3) {
		point postar = Illegal;
		for (int b = 0; b < 2; b++) {
			if ((enpos[i] - map.bonus_places[b]).len() < bonus_radius * 5) {
				postar = map.bonus_places[b];
				break;
			}
		}
		if (postar == Illegal) {
			if (enbelong == -1) {
				postar = crys[ally]; 
			}
			else {
				postar = targ[enemy];
			}
		}
		if (postar != Illegal && aux != Illegal) {
			okk = 1;
			for (int t = 0; t < 1 && 3 * (t + 2) < (ans - aux).len(); t++) {
				point ret = go_to(ans, postar);
				if (Legal(ret)) ans = ret;
				else break;
			}
		}
	}

	if (!okk) {
		/*	if (i == enbelong) {
			if (step != -1) {
			for ( ; step-- > 0; ) {
			point ret = go_to(ans, targ[enemy], 0.5);
			if (Legal(ret)) ans = ret;
			else break;
			}
			}
			else {
			for (int t = 2; (ans - aux).len() > 3 * t; t++) {
			point ret = go_to(ans, targ[enemy], 0.5);
			if (Legal(ret)) ans = ret;
			else break;
			}
			}
			}
			else */if (step != -1) {
				for ( ; step-- > 0; ) {
					point ret = ans + dir;
					if (Legal(ret)) ans = ret;
					else break;
				}
			}
			else {
				for (int t = 2; (ans - aux).len() > 3 * t; t++) {
					point ret = ans + dir;
					if (Legal(ret)) ans = ret;
					else break;
				}
			}
	}
	return ans;
}

int find_enemy(point x) {
	int att = -1;
	double dis = 1e9;
	for (int i = 0; i < hunum; i++) if (!endead[i]){
		go_to(x, enpos[i]);
		double tmp = last_go_to_dis;
		if (direct(x, enpos[i], 9999, 3) != Illegal) tmp *= 0.8;
		if (i == enbelong) tmp *= 0.8;
		tmp *= log(getHuman(enemy, i).hp);
		if (tmp < dis) att = i, dis = tmp;
	}
	return att;
}

point ff_enemy(point x) {
	int att = -1;
	double dis = 1e9;
	for (int j = 0; j < hunum; j++) if (!endead[j]){
		go_to(x, enpos[j]);
		double tmp = last_go_to_dis;
		if (tmp < dis) att = j, dis = tmp;
	}
	return att == -1 ? point(targ[enemy]) : enpos[att];
}


int has_enemy(point x, double r) {
	for (int j = 0; j < hunum; j++) if (!endead[j]) {
		double tmp = (enpos[j] - x).len();
		if (tmp < r + EP) return j;
	}
	return -1;
}

void get_bonus(int bonus_id, int p) {
	point b = map.bonus_places[bonus_id];
	if ((mypos[p] - b).len() < bonus_radius + EP) {
		is_guard[p] = 1;
		int tar = has_enemy(b, bonus_radius);
		dodge_ratio[p] = 6;
		if (tar != -1) {
			if (canflash[p] && canfire[p] && getHuman(enemy, tar).fire_time > 7 && getHuman(enemy, tar).flash_time > 4) {
				goal[p] = enpos[tar];
				goalr[p] = 6;
			}
			else {
				goal[p] = enpos[tar];
				goalr[p] = 2.5;
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

void suicide(int i, point ff = Illegal) {
	if (!Legal(ff)) ff = ff_enemy(mypos[i]); 
	goal[i] = ff;
	goalr[i] = 7;
}

void get_crystal(vector <int> squad) {
	// squad.size() == 3
	
	int alive = 0;
	for (auto v : squad) alive ++;

	point cp = crystal[enemy].position;
	bool far = (cp - targ[ally]).len() * 2 > (cp - targ[enemy]).len();
	if (far && (belonger == -1 || (ff_enemy(mypos[belonger]) - mypos[belonger]).len() < (targ[ally] - mypos[belonger]).len())) {
		for (auto i : squad) if (!mydead[i]) {
			if (i == belonger) {
				goal[i] = targ[ally];
			}
			else {
				goal[i] = cp;
				if (belonger != -1) {
					goalr[i] = fireball_radius * 2;
				}
				/*
				 for (int t = 0; t < rand_times; t++) {
					point x = cp + point(fireball_radius * 2, 0).turn(Rand(0, 359));
					if (Legal(x) && (abs((goal[i] - mypos[i]).len() - human_velocity) > abs((x - mypos[i]).len() - human_velocity) || !Legal(goal[i]))) {
						goal[i] = x;
					}
				}*/
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
				bool guard = 0;
				if (getHuman(ally, i).hp <= 15) {
					suicide(i);	
				}
			/*	else if (!guard) {
					goal[i] = cp;
					goalr[i] = fireball_radius * 2;
					guard = 1;
				}*/
				else {
					goal[i] = targ[enemy];
					point dir;
					for (int d = int((mypos[best] - targ[ally]).len() / 5 + 0.5);
						 d-- > 0 && Legal(dir = go_to(goal[i], cp, 4));
						 goal[i] = dir);
				}
			}
		}
	}

/*	for (int k = 0; (k <= 1 && (logic->frame < 800 || (logic->frame > 5500 && far))); k++) {
		int tar = has_enemy(map.bonus_places[mybonus ^ k], bonus_radius * 5);
		if ((logic->frame < 300 || tar != -1)) {
			int to = -1;
			for (auto v : squad) if (v != belonger) {
				if (to == -1 || (mypos[v] - map.bonus_places[mybonus ^ k]).len() * 2 - dis_to(mypos[v], goal[v]) < (mypos[to] - map.bonus_places[mybonus ^ k]).len() * 2 - dis_to(mypos[to], goal[to])) {
					to = v;
				}
			}
			if (tar == -1) suicide(to, map.bonus_places[mybonus ^ k]);
			else suicide(to, ff_enemy(map.bonus_places[mybonus ^ k]));
		}
	}
*/
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
	{
		double dis = -1;
		if (Legal(goal[id])) {
			go_to(x, goal[id]);
			dis = last_go_to_dis;
		}
		else {
			dis = (goal[id] - x).len();
		}

		bool nomove = 0;
		if (is_guard[id]) {
			bool flag = 0;
			for (int i = 0; i < 2; i++) {
				if ((goal[id] - (map.bonus_places[i])).len() < bonus_radius) {
					flag = 1;
					ret += dis * (dis > goalr[id] ? 1.5 : 0.01);
					ret += (map.bonus_places[i] - x).len() * 1;
					if (has_enemy(map.bonus_places[i], bonus_radius) != -1) {
						ret -= ((map.bonus_places[i] - x).len() < EP) * 0.1;
						ret -= ((enpos[has_enemy(map.bonus_places[i], bonus_radius)] - map.bonus_places[i]).len() > (x - map.bonus_places[i]).len()) * 0.3;
						nomove = 1;
					}
					break;
				}
			}
			if (!flag) {
				ret += abs(dis - goalr[id]);
			}
		}
		else {
			ret += abs(dis - goalr[id]);
		}

		if (!nomove) {
			ret -= (x - mypos[id]).len() * dodge_ratio[id] * .05;
		}
	}

	// away from ally
	if (nowfir.size() || nowmet.size()) {
		for (int i = 0; i < 5; i++) if (!mydead[i] && i != id) {
			double dis = (Move[i] - x).len();
			ret += max(0., (fireball_radius - dis)) * dodge_ratio[id] * .2;
		}
		// away from wall
		ret += max(0., fireball_radius * 2 - (x - point(disw[int(x.x)][int(x.y)])).len()) * dodge_ratio[id] * .1;
	}

	// away from enemy
	if (!is_guard[id]) for (int i = 0; i < 5; i++) if (!endead[i]) {
		double dis = (enpos[i] - x).len();
		ret += max(0., min(min(20 - dis, dis - 3), abs(dis - 6))) * .05 * dodge_ratio[id]; // danger zone
	}
	
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
				hurt = max(hurt, .3 * ((10 - t) / 8.) * (fireball_radius + human_velocity * 3 - dis));
			}
			nowr -= human_velocity * 0.9;
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
			ret += (nowr + human_velocity > dis ? 10 : 5) + pow(15 - v.last_time, 2) * (explode_radius - dis);
		}
		else if (dis < explode_radius +  human_velocity * 3 && !is_guard[id]) {
			ret += .3 * ((15 - v.last_time) / 5.) * (explode_radius + human_velocity * 3 - dis);
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

		for (auto &x : fireballs) {
			if (x.from_number % facnum != ally) {
				if ((x.position - Move[i]).len() < 50) {
					nowfir.push_back(x);
				}
			}
		}
		for (auto &x : meteors) {
			if (x.from_number % facnum != ally) {
				if ((x.position - Move[i]).len() < 50 && x.last_time < 15) {
					nowmet.push_back(x);
				}
			}
		}

		double sc = Score(i, Move[i]), tmp;

		if ((tmp = Score(i, mypos[i])) < sc) sc = tmp, Move[i] = mypos[i]; 

		point dir = point(human_velocity, 0);

		for (int d = 0; d < 360; d += 3) {
			point p = mypos[i] + dir.turn(d);
			tmp = Score(i, p);
			if (tmp < sc) sc = tmp, Move[i] = p;
		}

		for (int t = 0; t < rand_times; t++) {
			point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(1, 20) / 20.);
			tmp = Score(i, p);
			if (tmp < sc) sc = tmp, Move[i] = p;
		}
		if (is_guard[i]) {	
			for (int b = 0; b < 2; b++) if ((mypos[i] - map.bonus_places[b]).len() < bonus_radius) {
				double dis = (mypos[i] - map.bonus_places[b]).len();
				point p = map.bonus_places[b];
				if (dis > human_velocity) {
					p = mypos[i] * ((dis - human_velocity) / dis) + p * (human_velocity / dis);
				}
				logic->debugAppend(to_string(p.x) + ","+ to_string(p.y) + "\n");
				tmp = Score(i, p);
				logic->debugAppend(to_string(tmp) + ",");
				logic->debugAppend(to_string(sc));
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}

		if (!no_flash[i] && canflash[i]) {
			dir = dir.unit() * flash_distance;
			for (int d = 2; d < 360; d += 2) {
				point p = mypos[i] + dir.turn(d);
				tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			for (int t = 0; t < rand_times * 2; t++) {
				point p = mypos[i] + dir.turn(Rand(0, 359)) * (Rand(1, 40) / 40.);
				tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
			
			for (int b = 0; b < 2; b++) if ((mypos[i] - map.bonus_places[b]).len() < flash_distance) {
				point p = map.bonus_places[b];
				tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}

		if (goalr[i] > 0) {
			point p;
			for (int t = 0; t < rand_times * 2; t++) {
				if (is_guard[i]) {
					p = goal[i] + point(goalr[i], 0) * (Rand(1, 10) / 10.);
				}
				else {
					p = goal[i] + point(goalr[i], 0);
				}
				if ((p - mypos[i]).len() > human_velocity) continue;
				tmp = Score(i, p);
				if (tmp < sc) sc = tmp, Move[i] = p;
			}
		}

//		logic->debugAppend(to_string(i) + " : " + to_string(sc) + "\n");
	}

	for (int i = 0; i < hunum; i++) move(i, Move[i]);
}

void damage() {
	for (int i = 0; i < hunum; i++) attack[i] = find_enemy(Move[i]);
	for (int i = 0; i < hunum; i++) if (!mydead[i] && canfire[i] && !nofire[i])
		for (int j = i + 1; j < hunum; j++) if (!mydead[j] && canfire[j] && !nofire[j]) {
			double dis = (Move[i] - Move[j]).len(), endis = (Move[j] - ff_enemy(Move[j])).len();
			if (dis < 12 && endis > 20) {
				nofire[j] = 1;
			}
		}
	for (int i = 0; i < hunum; i++) if (!mydead[i] && canfire[i] && !nofire[i]) {
		if ((Move[i] - ff_enemy(Move[i])).len() > 20) {
			for (auto &x : fireballs) {
				if (x.from_number % facnum == ally) {
					if ((x.position - Move[i]).len() < 6) {
						nofire[i] = 1;
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < hunum; i++) {
		point my = Move[i], att = enpos[attack[i]];
		shoot(i, forecast(attack[i], -1, my));
		meteor(i, forecast(attack[i], meteor_delay));
		if (is_guard[i]) {
			meteor(i, map.bonus_places[0]);
			meteor(i, map.bonus_places[1]);
		}
	}
}

void solve() {
	time_t st = clock(); 

	logic = Logic::Instance();
	if (logic->frame == 1) init();	

	frame_before();

	get_bonus(0, mybonus);
//	get_bonus(1, 1);
	get_crystal({1, 2, 3, 4});
	adjust_movement();
	damage();	

	frame_after();
	
	logic->debugAppend("max. " + to_string(maxadj) + " \n");

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
