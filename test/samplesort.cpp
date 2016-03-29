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
#include "sequencedata.hpp"
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
  
using value_type = unsigned int;

void generate(size_t _nb, parray<value_type>& dst) {
  long n = _nb * 10000;
//  std::cerr << n << "\n";
  int r = quickcheck::generateInRange(0, 2); // currently something is wrong with exp_dist
//  std::cerr << r << "\n";
  if (r == 0) {
    dst = sequencedata::rand_int_range((value_type)0, (value_type)n, (value_type)INT_MAX);
  } else if (r == 1) {
    int m = quickcheck::generateInRange(0, 1 << 10);
    dst = sequencedata::almost_sorted<value_type>(0L, n, m);
  } else if (r == 2) {
    value_type x = (value_type)quickcheck::generateInRange(0, INT_MAX);
    dst = sequencedata::all_same(n, x);
  } else {
    dst = sequencedata::exp_dist<value_type>(0L, (value_type)n);
  }
}

void generate(size_t nb, container_wrapper<parray<value_type>>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */

using parray_wrapper = container_wrapper<parray<value_type>>;

class sorted_property : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& _in) {
    parray<value_type> a = _in.c;
    parray<value_type> b = _in.c;
    sample_sort(a.begin(), (int)a.size(), std::less<value_type>());
    std::sort(b.begin(), b.end(), std::less<value_type>());
    return same_sequence(a.cbegin(), a.cend(), b.cbegin(), b.cend());
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::sorted_property>(nb_tests, "samplesort is correct");
  });
  return 0;
}

/***********************************************************************/
