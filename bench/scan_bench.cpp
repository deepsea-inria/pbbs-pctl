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
#include <stdlib.h>
#include "bench.hpp"
#include "dpsdatapar.hpp"
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
      Item** tmps = new Item*[5];
      for (int i = 0; i < 5; i++) {
        tmps[i] = (Item*)malloc(sizeof(Item) * x.size());
        pbbs::sequence::scan(x.begin(), tmps[i], x.size(), [&] (Item x, Item y) { return x + y; }, (Item)0);
      }
    });
  } else {
    parray<parray<Item>> tmps(5);
    measured([&] {
      for (int i = 0; i < 5; i++) {
        tmps[i].prefix_tabulate(x.size(), 0);
        pasl::pctl::dps::scan(x.begin(), x.end(), (Item)0, [&] (Item x, Item y) { return x + y; }, tmps[i].begin(), pasl::pctl::forward_exclusive_scan);
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
