/*!
 * \file samplesort.cpp
 * \brief Example of sample sort usage
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "io.hpp"
#include "samplesort.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    void ex() {
      int n = pasl::util::cmdline::parse_or_default_int("n", 1000000);

      int* a = new int[n];
      for (int i = 0; i < n; i++) {
        a[i] = n - i;
      }
      sample_sort(a, n, std::less<int>());

      for (int i = 0; i < 10; i++) {
        printf("result[%d] = %d\n", i * n / 10, a[i * n / 10]);
      }
    }
    
  }
}

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    pasl::pctl::ex();
  });
  return 0;
}

/***********************************************************************/
