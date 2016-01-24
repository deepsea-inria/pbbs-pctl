/*!
 * \file pmap.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "io.hpp"
#include "pmap.hpp"

/***********************************************************************/

namespace pasl {
  namespace pctl {
    
    
    void ex() {
      
      //todo
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
