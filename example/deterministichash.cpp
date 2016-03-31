/*!
 * \file deterministichash.cpp
 * \brief Example of remove duplicates function usage
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "io.hpp"
#include "deterministichash.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    void ex() {
      int n = pasl::util::cmdline::parse_or_default_int("n", 1000000);

      srand(239);
      parray<int> a(2 * n);
      for (int i = 0; i < n; i++) {
        a[i] = i;
        a[n + i] = abs(rand()) % n;
      }
      parray<int> result = remove_duplicates(a);
      std::sort(result.begin(), result.end());

      std::cerr << result.size() << std::endl;
      for (int i = 0; i < 10; i++) {
        printf("result[%d] = %d\n", i * result.size() / 10, result[i * result.size() / 10]);
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
