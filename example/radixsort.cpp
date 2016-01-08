/*!
 * \file max.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "io.hpp"
#include "blockradixsort.hpp"
#include "seqradixsort.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    
    
    void ex() {
      
      parray<unsigned int> xs = { 3028713, 3798448, 4071828, 3028713 };
      parray<unsigned int> xs2(xs);
      integerSort(xs.begin(), (int)xs.size());
      std::sort(xs2.begin(),xs2.end());
      std::cout << "untrusted:\t" << xs << std::endl;
      std::cout << "trusted:\t" << xs2 << std::endl;
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
