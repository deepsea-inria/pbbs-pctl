/*!
 * \file max.cpp
 * \brief Quickcheck for reduce and scan
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "test.hpp"
#include "prandgen.hpp"
#include "max.hpp"

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

using value_type = int;

const value_type loval = 0;
const value_type hival = 1l<<20;

value_type random_value() {
  return quickcheck::generateInRange(loval, hival);
}

void generate(size_t nb, parray<value_type>& dst) {
  dst.resize(nb, loval);
  for (size_t i = 0; i < nb; i++) {
    dst[i] = random_value();
  }
}

void generate(size_t nb, pchunkedseq<value_type>& dst) {
  dst.clear();
  for (size_t i = 0; i < nb; i++) {
    dst.seq.push_back(random_value());
  }
}

void generate(size_t nb, container_wrapper<parray<value_type>>& c) {
  generate(nb, c.c);
}
  
/*---------------------------------------------------------------------*/
/* Trusted functions */
  
namespace trusted {

value_type max(const parray<value_type>& xs) {
  value_type m = std::numeric_limits<value_type>::lowest();
  for (long i = 0; i < xs.size(); i++) {
    m = std::max(m, xs[i]);
  }
  return m;
}

} // end namespace
  
/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
using parray_wrapper = container_wrapper<parray<value_type>>;
  
class flat_max_property : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& in) {
    return trusted::max(in.c) == max_ex::max(in.c);
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::flat_max_property>(nb_tests, "flat max is correct");
  });
  return 0;
}

/***********************************************************************/
