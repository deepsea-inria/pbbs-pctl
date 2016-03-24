/*!
 * \file scan.cpp
 * \brief Quickcheck for scan
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "test.hpp"
#include "prandgen.hpp"
#include "hull.hpp"
#include "geometryio.hpp"

/***********************************************************************/

namespace pasl {
namespace pctl {

/*---------------------------------------------------------------------*/
/* Quickcheck IO */

template <class Container>
std::ostream& operator<<(std::ostream& out, const container_wrapper<Container>& c) {
  out << c.c;
  return out;
}

/*---------------------------------------------------------------------*/
/* Quickcheck generators */
  
void generate(size_t _nb, parray<point2d>& dst) {
  intT nb = (intT)_nb;
  if (quickcheck::generateInRange(0, 1) == 0) {
    dst = plummer2d(nb);
  } else {
    bool inSphere = quickcheck::generateInRange(0, 1) == 0;
    bool onSphere = quickcheck::generateInRange(0, 1) == 0;
    dst = uniform2d(inSphere, onSphere, nb);
  }
}

void generate(size_t nb, container_wrapper<parray<point2d>>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
struct getX {
  point2d* P;
  getX(point2d* _P) : P(_P) {}
  double operator() (intT i) {return P[i].x;}
};

struct lessX {bool operator() (point2d a, point2d b) {
  return (a.x < b.x) ? 1 : (a.x > b.x) ? 0 : (a.y < b.y);} };

bool equals(point2d a, point2d b) {
  return (a.x == b.x) && (a.y == b.y);
}

//    if (f(v,r)) { r = v; k = j;}
bool check_hull(parray<point2d>& points,  parray<intT>& indices) {
  point2d* p = points.begin();
  intT n = points.size();
  intT hull_size = indices.size();
  point2d* convex_hull = newA(point2d, hull_size);
  for (intT i = 0; i < hull_size; i++) {
    convex_hull[i] = p[indices[i]];
  }
  intT idx = 0;
  for (int i = 0; i < hull_size; i++) {
    if (convex_hull[i].x > convex_hull[idx].x ||
      (convex_hull[i].x == convex_hull[idx].x && convex_hull[i].y > convex_hull[idx].y)) {
      idx = i;
    }
  }
  std::sort(p, p + n, lessX());
  if (!equals(p[0], convex_hull[0])) {
    cout << "checkHull: bad leftmost point" << endl;
    p[0].print();  convex_hull[0].print(); cout << endl;
    return 1;
  }
  if (!equals(p[n - 1], convex_hull[idx])) {
    cout << "checkHull: bad rightmost point" << endl;
    return 1;
  }
  intT k = 1;
  for (intT i = 0; i < idx; i++) {
    if (i > 0 && counter_clockwise(convex_hull[i - 1], convex_hull[i], convex_hull[i + 1])) {
      cout << "checkHull: not convex sides" << endl;
      return 1;
    }
    if (convex_hull[i].x > convex_hull[i + 1].x) {
      cout << "checkHull: upper hull not sorted by x" << endl;
      cout << indices << endl;
      for (int i = 0; i < hull_size; i++) {
        cout << convex_hull[i] << " ";
      }
      cout << std::endl;
      return 1;
    }
    while (k < n && !equals(p[k], convex_hull[i + 1]))
      if (counter_clockwise(convex_hull[i], convex_hull[i + 1], p[k++])) {
        cout << "checkHull: not convex" << endl;
        return 1;
      }
    if (k == n) {
      cout << "checkHull: unexpected points in hull" << endl;
      return 1;
    }
    k++;
  }
  k = n - 2;
  for (intT i = idx; i < hull_size - 1; i++) {
    if (i > idx && counter_clockwise(convex_hull[i - 1], convex_hull[i], convex_hull[i + 1])) {
      cout << "checkHull: not convex sides" << endl;
      return 1;
    }
    if (convex_hull[i].x < convex_hull[i + 1].x) {
      cout << "checkHull: lower hull not sorted by x" << endl;
      return 1;
    }
    while (k >= 0 && !equals(p[k], convex_hull[i + 1])) {
      if (counter_clockwise(convex_hull[i], convex_hull[i + 1], p[k--])) {
        cout << "checkHull: not convex" << endl;
      }
    }
    k--;
  }
  free(convex_hull);
  return 0;
}


using parray_wrapper = container_wrapper<parray<point2d>>;

class consistent_hulls_property : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& _in) {
    parray_wrapper in(_in);
    parray<intT> idxs = hull(in.c);
    return ! check_hull(in.c, idxs);
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::consistent_hulls_property>(nb_tests, "quickhull is correct");
  });
  return 0;
}

/***********************************************************************/
