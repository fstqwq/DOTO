#pragma once
#include <cmath>
#include "const.h"

namespace fstqwq {
	const int N = 320;
	const int M = 211;
	const int Bsiz = 16;
	const double eps = 1e-7;
	using std::abs;

	
	struct point {
		double x, y;
		point () {}
		point (double xx, double yy) : x(xx), y(yy) {}
		point (double xx[]) : x(xx[0]), y(xx[1]) {}
		point (Point a) : x(a.x), y(a.y) {} 
		point operator * (const double a) const {return point(x * a, y * a);}
		point operator / (const double a) const {return point(x / a, y / a);}
		friend point operator + (const point &a, const point &b) {return point(a.x + b.x, a.y + b.y);}
		friend point operator - (const point &a, const point &b) {return point(a.x - b.x, a.y - b.y);}
		double len() const {
			return sqrt(x * x + y * y);
		}
		bool operator == (const point &o) {
			return abs(x - o.x) < eps && abs(y - o.y) < eps;
		}
		bool operator != (const point &o) {
			return abs(x - o.x) >= eps || abs(y - o.y) >= eps;
		}
		operator Point () {
			return {x, y};
		}
	};


}
