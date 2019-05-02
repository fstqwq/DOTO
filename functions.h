#pragma once
#include "gamemap.h"
#include <cstdint>

namespace fstqwq {
	
	/* 
	 * In gamemap.h : 
	 * col[i][j] : color of (i, j)
	 * core[i] : core of color i
	 * ok[i][j] : if core[i] is directly reachable by core[j]
	 * g[i][j] : mid point shortest path
	*/
	
	const point Illegal(-1, -1);
	const point Gate[2] = {{105, 90}, {220, 245}};

	bool Legal(const point &x) {
		return x.x > 0 && x.x < N && x.y > 0 && x.y < N && col[int(x.x)][int(x.y)];
	}
	uint64_t xorshift128plus() {
		static uint64_t state[2] = {0x200312 + (uint64_t)rand(), 0xdeadbeef01234567};
		uint64_t t = state[0];
		uint64_t const s = state[1];
		state[0] = s;
		t ^= t << 23;		// a
		t ^= t >> 17;		// b
		t ^= s ^ (s >> 26);	// c
		state[1] = t;
		return t + s;
	}
	int Rand(int l, int r) {
		return l + int(xorshift128plus() % (r - l + 1)); 
	}

	point relative_pos(const point& st, const point& ed, double len) {
		return st + (ed - st).unit() * len;
	}
}
