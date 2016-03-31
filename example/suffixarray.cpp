/*!
 * \file suffixarray.cpp
 * \brief Example of suffix array
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "io.hpp"
#include "suffixarray.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    void ex() {
      int n = pasl::util::cmdline::parse_or_default_int("n", 1000000);

      parray<char> a(n);
      for (int i = 0; i < n; i++) {
        a[i] = 'a';
      }
      parray<int> result = suffix_array(a.begin(), n);

/*      std::cerr << result.size() << std::endl;
      for (int i = 0; i < result.size(); i++) {
        printf("result[%d] = %d\n", i, result[i]);
      }*/
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
