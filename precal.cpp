#include <bits/stdc++.h>
using namespace std;
typedef long long LL;
typedef long double LD;
#define mp(a, b) make_pair
#define x first
#define y second
#define read(x) scanf("%d", &x)
#define readl(x) scanf("%lld", &x)
#define readd(x) scanf("%lf", &x)

const int n = 320;
const int m = 211;

int dis[n][n];

unsigned char col[n][n];
double core[m][2];
int cnt[m];

void open_map() {
	FILE *Map = fopen("map.info", "rb");
	fread(col, 1, n * n, Map);
	fclose(Map);
}

void calc_dis() {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			core[col[i][j]][0] += i;
			core[col[i][j]][1] += j;
			cnt[col[i][j]]++;
		}
	for (int i = 1; i < m; i++) {
		core[i][0] /= cnt[i];
		core[i][1] /= cnt[i];
	}
}

struct point {
	double x, y;
	point () {}
	point (double xx, double yy) : x(xx), y(yy) {}
	point (double xx[]) : x(xx[0]), y(xx[1]) {}
	point operator * (const double a) const {return point(x * a, y * a);}
	point operator / (const double a) const {return point(x / a, y / a);}
	friend point operator + (const point &a, const point &b) {return point(a.x + b.x, a.y + b.y);}
	friend point operator - (const point &a, const point &b) {return point(a.x - b.x, a.y - b.y);}
	double len() const {
		return sqrt(x * x + y * y);
	}
};

point co[m];
bool ok[m][m];
double g[n][n];

void write_header() {
	FILE *header = fopen("gamemap.h", "w");
	fputs("#pragma once\n#include \"preset.h\"\nnamespace fstqwq {\n", header);
	fputs("unsigned char col[N][N] = {\n", header);
	for (int i = 0; i < n; i++) {
		fprintf(header, "{");
		for (int j = 0; j < n; j++) {
			fprintf(header, "%d%c", int(col[i][j]), j == n - 1 ? '}' : ',');
		}
		fprintf(header, "\n%c", i == n - 1 ? '}' : ',');
	}
	fprintf(header, ";\n");

	fputs("point core[M] = {\n", header);
	core[0][0] = core[0][1] = 0;
	for (int j = 0; j < m; j++) {
		fprintf(header, "{%.13lf, %.13lf}%c", core[j][0], core[j][1], j == m - 1 ? '}' : ',');
	}
	fprintf(header, "\n;\n");

	fputs("bool ok[M][M] = {\n", header);
	for (int i = 0; i < m; i++) {
		fprintf(header, "{");
		for (int j = 0; j < m; j++) {
			fprintf(header, "%d%c", int(ok[i][j]), j == m - 1 ? '}' : ',');
		}
		fprintf(header, "\n%c", i == m - 1 ? '}' : ',');
	}
	fprintf(header, ";\n");

	fputs("double g[M][M] = {\n", header);
	for (int i = 0; i < m; i++) {
		fprintf(header, "{");
		for (int j = 0; j < m; j++) {
			fprintf(header, "%.13lf%c", g[i][j], j == m - 1 ? '}' : ',');
		}
		fprintf(header, "\n%c", i == m - 1 ? '}' : ',');
	}
	fprintf(header, ";\n");


	fputs("int disw[N][N] = {\n", header);
	for (int i = 0; i < n; i++) {
		fprintf(header, "{");
		for (int j = 0; j < n; j++) {
			fprintf(header, "%d%c", dis[i][j], j == n - 1 ? '}' : ',');
		}
		fprintf(header, "\n%c", i == n - 1 ? '}' : ',');
	}
	fprintf(header, ";\n");


	fprintf(header, "}\n");

	fclose(header);

}

void calc_map() {
	for (int i = 1; i < m; i++) co[i] = point(core[i]);
	for (int i = 0; i < m ;i++) {
		for (int j = 0; j < m; j++) {
			g[i][j] = 0;
		}
	}
	for (int i = 1; i < m ;i++) {
		for (int j = 1; j < m; j++) {
			g[i][j] = 1e12;
		}
		ok[i][i] = 1;
		g[i][i] = 1; // stay is expensive
	}
	for (int i = 1; i < m; i++)
		for (int j = i + 1; j < m; j++) {
			bool flag = 1;
			for (int ra = 1; ra < 1000; ra++) {
				point o = (co[i] * (ra / 1000.)) + (co[j] * ((1000 - ra) / 1000.));  
				int x = o.x, y = o.y;
				if (!col[x][y]) {
					flag = 0;
					break;
				}
			}
			if ((ok[j][i] = ok[i][j] = flag)) {
				g[i][j] = g[j][i] = (co[i] - co[j]).len();
			}
		}
	for (int k = 1; k < m; k++)
		for (int i = 1; i < m; i++)
			for (int j = 1; j < m; j++) {
				g[i][j] = min(g[i][j], g[i][k] + g[k][j]);
			}
}
void calc_wall() {
	const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
	queue <pair<int,int> > q;
	memset(dis, 0x3f, sizeof dis);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			if (!col[i][j]) {
				dis[i][j] = 0;
				q.push({i, j});
			}
		}
	while (!q.empty()) {
		auto o = q.front(); q.pop();
		int x = o.x, y = o.y;
		for (int d = 0; d < 4; d ++) {
			int tx = x + dx[d], ty = y + dy[d];
			if (tx >= 0 && tx < n && ty >= 0 && ty < n && dis[tx][ty] > dis[x][y] + 1) {
				dis[tx][ty] = dis[x][y] + 1;
				q.push({x, y});
			}
		}
	}
}

int main() {
	open_map();
	calc_dis();
	calc_map();
	calc_wall();
	write_header();
}
