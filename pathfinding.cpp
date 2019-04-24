#pragma once
#include "functions.h"
#include "const.h"
#include <algorithm>

#include <iostream>
using namespace std;


namespace fstqwq {
	using std::min;
	using std::max;
	// if ed is directly reachable by st
	// time complexity : (st - ed).len() / 0.6, appox. 500
	point direct(const point &st, const point &ed) {
		double dis = (st - ed).len();
		if (!Legal(st) || !Legal(ed)) return Illegal;
		for (double d = CONST::human_velocity; d < dis; d += CONST::human_velocity) {
			point p = st * ((dis - d) / dis) + ed * (d / dis);
			if (!Legal(p)) return Illegal;
		}
		return dis <= CONST::human_velocity ? ed : (st * ((dis - CONST::human_velocity) / dis) + ed * (CONST::human_velocity / dis));
	}

	// TODO : priority flash
	// if ed is directly reachable by st start with a flash
	// time complexity : (st - ed).len() / 0.6, appox. 500 * 2
	point direct_flash(const point &st, const point &ed) {
		double dis = (st - ed).len();
		if (!Legal(st) || !Legal(ed)) return Illegal;
		bool flashed = 0;
		for (double d = CONST::human_velocity; d < dis; d += CONST::human_velocity) {
			point p = st * ((dis - d) / dis) + ed * (d / dis);
			if (!Legal(p)) {
				if (flashed) return Illegal;
				d -= 0.6;
				d = min(d + CONST::flash_distance, dis) - CONST::human_velocity;
			}
		}

		point ret = (st * ((dis - CONST::human_velocity) / dis) + ed * (CONST::human_velocity / dis));
		return dis <= CONST::human_velocity ? ed : (Legal(ret) ? ret : st * ((dis - min(dis, CONST::flash_distance)) / dis) + ed * ((dis - min(dis, CONST::flash_distance)) / dis));
	}

	// Find a path from st to ed if without fireballs
	// time complexity : appox. 500 * 100 * Luck(0, 1)
	double last_go_to_dis;

	point go_to(const point &st, const point &ed) {
		double &ans = last_go_to_dis;	

		point ret = direct(st, ed);
		if (ret != Illegal) {
			ans = (ed - st).len(); 
			return ret;
		}

		point pos = Illegal;
		int colst = col[int(st.x)][int(st.y)], coled = col[int(ed.x)][int(ed.y)];
		ans = 1e10;

		for (int i = 1; i < M; i++) if (ok[colst][i] || (core[i] - st).len() <= Bsiz) {
			double tmp1 = (st - core[i]).len(), tmp2 = g[i][coled];
			if (tmp1 != 0 && tmp1 + tmp2 < ans && (ret = direct(st, core[i])) != Illegal) {
				pos = ret, ans = tmp1 + tmp2;
			}
		}
	
		return pos;
	}

	// TODO : Rush TO
}

