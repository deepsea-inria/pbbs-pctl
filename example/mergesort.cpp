/*!
 * \file mergesort.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "example.hpp"
#include "psort.hpp"
#include "io.hpp"

/***********************************************************************/

namespace pasl {
namespace pctl {

void ex() {
  
  // parallel array
  std::cout << "-- parray -- " << std::endl;
  {
    parray<int> xs = { 3, 2, 100, 1, 0, -1, -3 };
    std::cout << "xs\t\t\t=\t" << xs << std::endl;
    pasl::pctl::sort(xs.begin(), xs.end(), std::less<int>());
    std::cout << "mergesort(xs)\t=\t" << xs << std::endl;
  }
  std::cout << std::endl;
  
  // parallel chunked sequence
  std::cout << "-- pchunkedseq -- " << std::endl;
  {
    pchunkedseq<int> xs = { 3, 2, 100, 1, 0, -1, -3 };
    std::cout << "xs\t\t\t=\t" << xs << std::endl;
    std::cout << "mergesort(xs)\t=\t" << pchunked::sort(xs, std::less<int>()) << std::endl;
  }
  std::cout << std::endl;

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
