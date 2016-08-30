/*!
 * \file maxindex.cpp
 * \brief Example of maximum index find usage
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <functional>
#include "example.hpp"
#include "io.hpp"
#include "dpsdatapar.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    void ex() {
      int n = pasl::util::cmdline::parse_or_default_int("n", 1000);
      int* x = new int[n];
      for (int i = 0; i < n; i++) {
        x[i] = i;
      }

      auto greater = [&] (int x, int y) {
        return x > y;
      };

      long index = max_index(x, x + n, 0, greater, [&] (long i, int) {
        return n - x[i];
      });
      std::cerr << index << std::endl;
    }
  }
}


 int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    pasl::pctl::ex();
  });
  return 0;
}
    
