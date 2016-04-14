/*!
 * \file psort.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <math.h>
#include <functional>
#include "bench.hpp"
#include "samplesort.hpp"
#include "loaders.hpp"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = pasl::util::cmdline::parse_or_default_int("test", 1);
    if (test == 0) {
      parray<int> a = pasl::pctl::io::load_random_seq<int>("random_seq_10000000", 10000000);
      measured([&] {
        pasl::pctl::sample_sort(a.begin(), (int)a.size(), std::less<int>());
      });
    }
  });
  return 0;
}

/***********************************************************************/
