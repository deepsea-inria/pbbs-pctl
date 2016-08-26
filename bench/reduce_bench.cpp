/*!
 * \file reduce_bench.cpp
 * \brief Benchmarking script for parallel reduce
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <math.h>
#include <functional>
#include <stdlib.h>
#include "bench.hpp"
#include "datapar.hpp"
#include "loaders.hpp"
#include "sequence.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

template <class Item, class Compare_fct>
void pbbs_pctl_call(pbbs::measured_type measured, parray<Item>& x, const Compare_fct& compare) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    measured([&] {
      for (int i = 0; i < 30; i++) {
        pbbs::sequence::reduce(x.begin(), (int)x.size(), [&] (Item x, Item y) { return x + y; });
      }
    });
  } else {
    measured([&] {
      for (int i = 0; i < 30; i++) {
        pasl::pctl::reduce(x.cbegin(), x.cend(), 0, [&] (Item x, Item y) { return x + y; });
      }
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default<std::string>("infile", "");
    if (infile != "") {
      deepsea::cmdline::dispatcher d;
      d.add("array_double", [&] {
        parray<double> x = pasl::pctl::io::load<parray<double>>(infile);
        pbbs_pctl_call(measured, x, std::less<double>());
      });
      d.add("array_int", [&] {
        parray<int> x = pasl::pctl::io::load<parray<int>>(infile);
        pbbs_pctl_call(measured, x, std::less<int>());
      });
      d.dispatch("type");
      return;
    }

  });
  return 0;
}

/***********************************************************************/
