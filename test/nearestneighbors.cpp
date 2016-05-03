/*!
 * \file scan.cpp
 * \brief Quickcheck for scan
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <limits>
#include <float.h>

#include "test.hpp"
#include "prandgen.hpp"
#include "geometrydata.hpp"
#include "nearestneighbors.hpp"
#include "samplesort.hpp"

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
  
using intT = int;

void generate(size_t _nb, parray<point2d>& dst) {
  intT nb = (intT)_nb * 10;
  std::cerr << "Problem size: " << nb << std::endl;
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

template <class pointT>
int check_neighbours(parray<intT>& neighbours, pointT* p, intT n, intT k) {
  if (neighbours.size() != k * n) {
    cout << "error in neighboursCheck: wrong length, n = " << n
    << " k = " << k << " neighbours = " << neighbours.size() << endl;
    return 1;
  }
  
  int result = 0;
  parallel_for((intT)0, n, [&] (intT j) {
    double* distances = newA(double, n);
    parallel_for ((intT)0, n, [&] (intT i) {
      if (i == j) {
        distances[i] = std::numeric_limits<double>().max();
      } else {
        distances[i] = (p[j] - p[i]).length();
      }
    });
    sample_sort(distances, n, std::less<double>());
    
    double error_tolerance = 1e-6;
    for (int i = 0; i < std::min(k, n - 1); i++) {
      double d = (p[j] - p[neighbours.begin()[k * j + i]]).length();
      double curd = distances[i];

      if ((d - curd) / (d + curd)  > error_tolerance) {
        cout << "error in neighboursCheck: for point " << j
        << " " << k << "-th min distance reported is: " << d
        << " actual is: " << curd << endl;
        result = 1;
      }
    }
  });
  return result;
}
  
using parray_wrapper = container_wrapper<parray<point2d>>;

template <class point, int maxK>
class nearestneighbours_property : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& _in) {
    using vertex = vertex<point, maxK>;
    parray_wrapper in(_in);
    intT n = (intT)in.c.size();
    parray<point>& points = in.c;
    int k = 10;
    parray<int> result = ANN<intT, maxK, point>(points, n, k);

    int r = 10;
    return !check_neighbours(result, points.begin(), n, k);
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = deepsea::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::nearestneighbours_property<point2d, 10>>(nb_tests, "nearestneighbours is correct");
  });
  return 0;
}

/***********************************************************************/
