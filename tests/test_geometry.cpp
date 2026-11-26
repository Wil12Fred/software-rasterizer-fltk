// Unit tests for the non-GUI parts of the rasterizer: the 2-D algorithms in
// renderer/geometry.h (Bresenham, scan-line fill, Cohen-Sutherland and
// Sutherland-Hodgman clipping) and the 4x4 math in math/euclidean.
// FLTK is not needed: geometry.h and the math code do not include it.
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <set>
#include <utility>
#include "vector.h"
#include "matrix.h"
#include "geometry.h"
#include "euclidean.h"
#include "transformation.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)

// ---------------------------------------------------------- Bresenham
// Lines into every octant from (20,20): both endpoints are drawn, one pixel
// per step of the major axis, 8-connected, never more than half a pixel off
// the ideal line.
static void test_bresenham_all_octants() {
  const int ends[][2] = {{30, 24}, {24, 30}, {16, 30}, {10, 24}, {10, 16}, {16, 10}, {24, 10}, {30, 16},
                         {35, 20}, {20, 35}, {5, 20}, {20, 5}, {30, 30}, {10, 10}, {30, 10}, {10, 30}};
  for (unsigned t = 0; t < sizeof ends / sizeof ends[0]; t++) {
    Point A(20, 20), B(ends[t][0], ends[t][1]);
    Bresenham br(Line(A, B));
    int dx = B.x - A.x, dy = B.y - A.y;
    int major = std::max(std::abs(dx), std::abs(dy));
    CHECK(br.size == major + 1);
    std::set<std::pair<int, int> > pts;
    for (int i = 0; i < br.size; i++) pts.insert(std::make_pair(br.points[i].x, br.points[i].y));
    CHECK((int)pts.size() == br.size);                       // no pixel twice
    CHECK(pts.count(std::make_pair(A.x, A.y)) && pts.count(std::make_pair(B.x, B.y)));
    for (int i = 0; i < br.size; i++) {
      double px = br.points[i].x - A.x, py = br.points[i].y - A.y;
      double dist = std::fabs(px * dy - py * dx) / std::sqrt(double(dx * dx + dy * dy));
      CHECK(dist <= 0.5 + 1e-9);
    }
    for (int i = 1; i < br.size; i++)
      CHECK(std::abs(br.points[i].x - br.points[i-1].x) <= 1 && std::abs(br.points[i].y - br.points[i-1].y) <= 1);
  }
}

// -------------------------------------------------- Cohen-Sutherland
static const Point Base(0, 0), Top(100, 50);   // window [0,100) x [0,50)
static bool inside(const Point& p) { return p.x >= Base.x && p.x < Top.x && p.y >= Base.y && p.y < Top.y; }

static void test_cohen_sutherland() {
  Color c(1, 1, 1);
  {   // fully inside: accepted untouched
    CohenSutherland cs(Line(Point(10, 10, c), Point(60, 40, c)));
    CHECK(cs.clipping(Base, Top));
    CHECK(cs.L.F.x == 10 && cs.L.F.y == 10 && cs.L.S.x == 60 && cs.L.S.y == 40);
  }
  {   // fully to the left: rejected
    CohenSutherland cs(Line(Point(-30, 10, c), Point(-5, 40, c)));
    CHECK(!cs.clipping(Base, Top));
  }
  {   // crossing the top and the right edge: clipped to the window, on the line
    Point A(50, 25, c), B(150, 75, c);       // y = 25 + (x - 50) / 2
    CohenSutherland cs(Line(A, B));
    CHECK(cs.clipping(Base, Top));
    CHECK(inside(cs.L.F) && inside(cs.L.S));
    const Point* e[2] = {&cs.L.F, &cs.L.S};
    for (int i = 0; i < 2; i++) CHECK(std::abs(e[i]->y - (25 + (e[i]->x - 50) / 2.0)) <= 1);
    CHECK(std::max(cs.L.F.x, cs.L.S.x) >= 96);  // reaches the boundary it crosses (y = 49)
  }
  {   // crossing the left edge
    CohenSutherland cs(Line(Point(-20, 10, c), Point(40, 10, c)));
    CHECK(cs.clipping(Base, Top));
    CHECK(std::min(cs.L.F.x, cs.L.S.x) == 0 && std::max(cs.L.F.x, cs.L.S.x) == 40);
  }
}

// -------------------------------------------------------- scan line
static std::vector<Line> polygon(const std::vector<Point>& v) {
  std::vector<Line> e;
  for (size_t i = 0; i < v.size(); i++) e.push_back(Line(v[i], v[(i + 1) % v.size()]));
  return e;
}

static void test_scanline_rectangle() {
  std::vector<Point> v;
  v.push_back(Point(10, 5)); v.push_back(Point(30, 5)); v.push_back(Point(30, 15)); v.push_back(Point(10, 15));
  std::vector<Line> e = polygon(v);
  std::vector<Point> px;
  ScanLine sl((int)e.size(), &e[0]);
  sl.draw_points(px);
  std::set<std::pair<int, int> > s;
  for (size_t i = 0; i < px.size(); i++) s.insert(std::make_pair(px[i].x, px[i].y));
  // rows 5..14 (the top edge is exclusive), columns 10..30
  CHECK(s.size() == 10u * 21u);
  for (int y = 5; y < 15; y++) for (int x = 10; x <= 30; x++) CHECK(s.count(std::make_pair(x, y)));
}

static void test_scanline_triangle() {
  std::vector<Point> v;
  v.push_back(Point(0, 0)); v.push_back(Point(40, 0)); v.push_back(Point(0, 40));
  std::vector<Line> e = polygon(v);
  std::vector<Point> px;
  ScanLine sl((int)e.size(), &e[0]);
  sl.draw_points(px);
  std::set<std::pair<int, int> > s;
  for (size_t i = 0; i < px.size(); i++) {
    s.insert(std::make_pair(px[i].x, px[i].y));
    CHECK(px[i].x >= 0 && px[i].y >= 0 && px[i].x + px[i].y <= 40);   // never outside
  }
  if (s.size() != 860) std::fprintf(stderr, "triangle pixels: %d\n", (int)s.size());
  CHECK(s.size() == 860);   // rows 0..39, x from 0 to 40 - y inclusive: sum of (41 - y)
}

// ------------------------------------------------- Sutherland-Hodgman
static void test_sutherland_hodgman() {
  Color c(1, 0, 0);
  std::vector<Point> v;   // a square sticking out of the window on the right and the top
  v.push_back(Point(60, 20, c)); v.push_back(Point(140, 20, c)); v.push_back(Point(140, 90, c)); v.push_back(Point(60, 90, c));
  std::vector<Line> e = polygon(v);
  SutherlandHodgeman sh((int)e.size(), &e[0]);
  Line* out = 0;
  int n = sh.clipping(Base, Top, out);
  CHECK(n >= 4);
  for (int i = 0; i < n; i++) CHECK(inside(out[i].F) && inside(out[i].S));
  // and the clipped outline still fills the visible part: rows 20..48, columns 60..99
  std::vector<Point> px;
  ScanLine sl(n, out);
  sl.draw_points(px);
  std::set<std::pair<int, int> > s;
  for (size_t i = 0; i < px.size(); i++) s.insert(std::make_pair(px[i].x, px[i].y));
  CHECK(s.count(std::make_pair(60, 20)) && s.count(std::make_pair(99, 20)) && s.count(std::make_pair(80, 40)));
  for (std::set<std::pair<int, int> >::iterator it = s.begin(); it != s.end(); ++it)
    CHECK(it->first >= 60 && it->first < 100 && it->second >= 20 && it->second < 50);
}

// ------------------------------------------------------ 4x4 math
static void test_cross_and_invert() {
  vector<3> x(1, 0, 0), y(0, 1, 0), z;
  cross(x, y, z);
  CHECK(z[0] == 0 && z[1] == 0 && z[2] == 1);

  transformation t;
  vector<3> d(3, -2, 5), axis(0, 0, 1), s(2, 2, 2);
  t.translate(d);
  t.rotate(30, axis);
  t.scale(s);
  matrix<4, 4> m = t.t(), inv = t.t();
  invert(inv);
  matrix<4, 4> id = m * inv;
  for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) CHECK(std::fabs(id[i][j] - (i == j ? 1.f : 0.f)) < 1e-4);
}

static void test_translate_applies() {
  transformation t;
  vector<3> d(3, -2, 5);
  t.translate(d);
  vector<4> p(1, 1, 1, 1);
  t.apply(p);
  CHECK(std::fabs(p[0] - 4) < 1e-5 && std::fabs(p[1] + 1) < 1e-5 && std::fabs(p[2] - 6) < 1e-5 && std::fabs(p[3] - 1) < 1e-5);
}

int main() {
  test_bresenham_all_octants();
  test_cohen_sutherland();
  test_scanline_rectangle();
  test_scanline_triangle();
  test_sutherland_hodgman();
  test_cross_and_invert();
  test_translate_applies();
  if (failures) { std::printf("%d check(s) failed\n", failures); return 1; }
  std::printf("all geometry tests passed\n");
  return 0;
}
