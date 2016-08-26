/*!
 * \file raycast.cpp
 * \brief Quickcheck for ray cast
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "test.hpp"
#include "prandgen.hpp"
#include "kdtree.hpp"
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
  
class test_case {
public:
  parray<pointT> points;
  parray<triangle> triangles;
  parray<ray<pointT>> rays;
};

std::ostream& operator<<(std::ostream& out, const test_case& c) {
  return out;
}

void generate(size_t _nb, test_case& dst) {
  intT nb = (intT)10 * _nb;
  printf("Test with n = %d\n", nb);

  bool onSphere = quickcheck::generateInRange(0, 1) == 0;

  dst.points.resize(3 * nb);
  dst.triangles.resize(nb);
  double d = 1.0 / sqrt((double) nb);
  pasl::pctl::parallel_for(0, nb, [&] (int i) {
    if (onSphere) {
      dst.points[3 * i] = randOnUnitSphere3d<int, unsigned int>((i + 1) * nb);
    } else {
      dst.points[3 * i] = rand3d<int, unsigned int>((i + 1) * nb);
    }
    dst.points[3 * i + 1] = dst.points[3 * i] + vect3d(d, d, 0);
    dst.points[3 * i + 2] = dst.points[3 * i] + vect3d(d, 0, d);
    dst.triangles[i].vertices[0] = 3 * i;
    dst.triangles[i].vertices[1] = 3 * i + 1;
    dst.triangles[i].vertices[2] = 3 * i + 2;
  });

  dst.rays.resize(nb);
  pointT p0 = dst.points[0];
  pointT p1 = dst.points[0];
  for (int i = 0; i < nb; i++) {
    p0 = p0.min_coord(dst.points[i]);
    p1 = p1.max_coord(dst.points[i]);
  }

  vect3d shift = p1 - p0;
  // parallel for seems to break
  for (intT i = 0; i < nb; i++) {
    pointT pl(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 0), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 1), 
		 p0.z);
    pointT pr(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 2), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 3), 
		 p1.z);
    dst.rays[i] = ray<pointT>(pl, pr - pl);
  }

}

void generate(size_t nb, container_wrapper<test_case>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
using test_case_wrapper = container_wrapper<test_case>;

class consistent_hulls_property : public quickcheck::Property<test_case_wrapper> {
public:
  
  bool holdsFor(const test_case_wrapper& _in) {
    triangles<pointT> tri(_in.c.points.size(), _in.c.triangles.size(), _in.c.points.begin(), _in.c.triangles.begin());
    parray<intT> original = pasl::pctl::kdtree::ray_cast(tri, _in.c.rays.begin(), _in.c.rays.size());
    parray<intT> seq = pasl::pctl::kdtree::ray_cast(tri, _in.c.rays.begin(), _in.c.rays.size());
    for (int i = 0; i < original.size(); i++) {
//      std::cerr << seq << std::endl;
      if (original[i] != seq[i]) {
        printf("The triangle for ray %d has been found incorrectly, %d against %d\n", i, original[i], seq[i]);
        return false;
      }
    }
    return true;
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = deepsea::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::consistent_hulls_property>(nb_tests, "quickhull is correct");
  });
  return 0;
}

/***********************************************************************/
