#pragma once
#include "functions.h"
#include "const.h"
#include "logic.h"
#include <algorithm>

//#include <iostream>
//using namespace std;


namespace fstqwq {
	using std::min;
	using std::max;

	// if ed is directly reachable by st
	// time complexity : (st - ed).len() / 0.6, appox. 500
	point direct(const point &st, const point &ed, int step_limit = 9999, double step = CONST::human_velocity) {
		double dis = (st - ed).len();
		if (!Legal(st) || !Legal(ed)) return Illegal;
	/*	for (double d = CONST::human_velocity; d < dis && step_limit--; d += CONST::human_velocity) {
			point p = st * ((dis - d) / dis) + ed * (d / dis);
			if (!Legal(p)) return Illegal;
		}*/

		point p = st, dir = (st * ((dis - step) / dis) + ed * (step / dis)) - st;
		for (double d = step; d < dis && step_limit--; d += step) {
			p = p + dir;
			if (!Legal(p)) return Illegal;
		}

		return dis <= step ? ed : (st * ((dis - step) / dis) + ed * (step / dis));
	}

	// Find a path from st to ed if without fireballs
	// time complexity : appox. 500 * 100 * Luck(0, 1)
	double last_go_to_dis;

	point go_to(const point &st, const point &ed, double step = CONST::human_velocity) {
		double &ans = last_go_to_dis;	

		ans = 1e10;
		if (!Legal(st) || !Legal(ed)) return Illegal;

		point ret = direct(st, ed, 9999, step);
		if (ret != Illegal) {
			ans = (ed - ret).len(); 
			return ret;
		}

		point pos = Illegal;
		int colst = col[int(st.x)][int(st.y)], coled = col[int(ed.x)][int(ed.y)];

		for (int i = 1; i < M; i++) if (ok[colst][i] || (core[i] - st).len() <= Bsiz * 3 / 2) {
			double tmp1 = max(0., (st - core[i]).len() - step), tmp2 = g[i][coled];
			//TODO: to be tested if optimized
			//XXX : ok
			if (tmp1 != 0 && tmp1 + tmp2 < ans && (ret = direct(st, core[i], 99, step)) != Illegal) {
				pos = ret, ans = tmp1 + tmp2;
			}
		}
	
		return pos;
	}
	
	point go_to(const Human &x, const point &ed) {
		return go_to(x.position, ed);
	}

	double dis_to(const point &st, const point &ed) {
		go_to(st, ed);
		return last_go_to_dis;
	}
	double last_rush_to_ans;
	// We have to flash as fast as we could
	point rush_to(const point &st, const point &ed) {
		double &ans = last_rush_to_ans;
		
		double dis = (ed - st).len();
		if (dis > human_velocity && dis <= CONST::flash_distance) return ans = 0, ed;

		point std_ans = go_to(st, ed);
		double std_dis = last_go_to_dis;
		point after_jump = ed * (CONST::flash_distance / dis) + st * ((dis - CONST::flash_distance) / dis);

		if (go_to(after_jump, ed) != Illegal && last_go_to_dis + CONST::human_velocity < std_dis) {
			return ans = last_go_to_dis, after_jump;
		}
		else {
			return ans = std_dis, std_ans;
		}
	}

	point rush_to(const Human &x, const point &ed) {
		return rush_to(x.position, ed);	
	}
}

