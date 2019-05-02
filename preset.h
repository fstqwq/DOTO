#pragma once
#include <cmath>
#include "const.h"
#include "geometry.h"
#include "logic.h"

namespace fstqwq {
	const int N = 320;
	const int M = 211;
	const int Bsiz = 16;
	const double pi = 3.14159265358979323846264338;
	const double eps = 1e-7;
	using std::abs;

	
	struct point {
		double x, y;
		point () {}
		point (double xx, double yy) : x(xx), y(yy) {}
		point (double xx[]) : x(xx[0]), y(xx[1]) {}
		point (const Point& a) : x(a.x), y(a.y) {} 
		point (const Human& a) : x(a.position.x), y(a.position.y) {}
		template <class U, class V> point (const pair <U, V>& a) {
			x = a.first; y = a.second;
		}
		point operator * (const double a) const {return point(x * a, y * a);}
		point operator / (const double a) const {return point(x / a, y / a);}
		friend point operator + (const point &a, const point &b) {return point(a.x + b.x, a.y + b.y);}
		friend point operator - (const point &a, const point &b) {return point(a.x - b.x, a.y - b.y);}
		double len() const {
			return sqrt(x * x + y * y);
		}
		point unit() const {
			if (abs(x) < eps && abs(y) < eps) return {0, 0};
			return *this / len();
		}
		bool operator == (const point &o) const {
			return abs(x - o.x) < eps && abs(y - o.y) < eps;
		}
		bool operator != (const point &o) const {
			return abs(x - o.x) >= eps || abs(y - o.y) >= eps;
		}
		operator Point () const {
			return {x, y};
		}
		point turn(double theta) const {
			return point(x * cos(theta) - y * sin(theta), x * sin(theta) + y * cos(theta));
		}
		point turn(int degree) const {
			double theta = degree / 180. * pi;
			return turn(theta);
		}
	};


}
