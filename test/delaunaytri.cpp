/*!
 * \file delaunaytri.cpp
 * \brief Quickcheck for delaunary triangulation
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <set>

#include "test.hpp"
#include "prandgen.hpp"
#include "geometrydata.hpp"
#include "delaunay.hpp"
#include "topology.hpp"

/***********************************************************************/

namespace pasl {
namespace pctl {

/*---------------------------------------------------------------------*/
/* Quickcheck IO */

template <class Container>
std::ostream& operator<<(std::ostream& out, const container_wrapper<Container>& c) {
  // out << c.c;
  return out;
}

/*---------------------------------------------------------------------*/
/* Quickcheck generators */


void generate(size_t _nb, parray<point2d>& dst) {
  intT nb = (intT)_nb;
  if (quickcheck::generateInRange(0, 1) == 0) {
    dst = plummer2d(nb);
  } else {
    dst = uniform2d(true, false, nb);
  }
}

void generate(size_t nb, container_wrapper<parray<point2d>>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
bool dcheck(triangles<point2d> Tri, parray<point2d>& P) {
  intT m = Tri.numTriangles;
  for (intT i=0; i < P.size(); i++)
    if (P[i].x != Tri.P[i].x || P[i].y != Tri.P[i].y) {
      cout << "checkDelaunay: prefix of points don't match input at "
      << i << endl;
      cout << P[i] << " " << Tri.P[i] << endl;
      return 0;
    }
  vertex* V = NULL;
  tri* Triangs = NULL;
  exit(0);
  //todo: uncomment below and fix type error
  //topologyFromTriangles(Tri, &V, &Triangs);
  return checkDelaunay(Triangs, m, 10);
}

using parray_wrapper = container_wrapper<parray<point2d>>;

class prop : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& _in) {
    parray_wrapper in(_in);
    triangles<point2d> tri = delaunay(in.c.begin(), in.c.size());
    return ! dcheck(tri, in.c);
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::prop>(nb_tests, "delaunay triangulation is correct");
  });
  return 0;
}

/***********************************************************************/
