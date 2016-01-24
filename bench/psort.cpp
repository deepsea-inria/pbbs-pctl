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

#include "bench.hpp"
#include "psort.hpp"
#include "prandgen.hpp"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using pchunkedseq = pasl::pctl::pchunkedseq<Item>;
template <class Item>
using parray = pasl::pctl::parray<Item>;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    long n = pasl::util::cmdline::parse_or_default_long("n", 100000);
    long m = pasl::util::cmdline::parse_or_default_long("m", 200*n);
    std::string datastruct = pasl::util::cmdline::parse_or_default_string("datastruct", "pchunkedseq");
    if (datastruct == "pchunkedseq") {
      pchunkedseq<int> xs = pasl::pctl::prandgen::gen_integ_pchunkedseq(n, 0, (int)m);
      measured([&] {
        xs = pasl::pctl::pchunked::mergesort(xs, std::less<int>());
      });
      std::cout << "result\t" << xs.seq[0] << std::endl;
      assert(xs.seq.size() == n);
    } else if (datastruct == "parray") {
      parray<int> xs = pasl::pctl::prandgen::gen_integ_parray(n, 0, (int)m);
      measured([&] {
        pasl::pctl::mergesort(xs.begin(), xs.end(), std::less<int>());
      });
      std::cout << "result\t" << xs[0] << std::endl;
      assert(xs.size() == n);
    } else {
      exit(0);
    }
  });
  return 0;
}

/***********************************************************************/
