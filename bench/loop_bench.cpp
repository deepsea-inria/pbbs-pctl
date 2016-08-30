/*!
 * \file loop_bench.cpp
 * \brief Benchmarking script for nested parallel loops
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

constexpr char nested_loop_file[] = "nested_loop";

pasl::pctl::granularity::estimator* outer_estimator = NULL;
pasl::pctl::granularity::estimator* inner_estimator = NULL;

template <class Cmp_fct, class Body_fct>
void parallel_for(int l, int r, const Cmp_fct& cmp_fct, const Body_fct& body) {
  double comp = cmp_fct(l, r);
  using controller_type = pasl::pctl::granularity::controller_holder<nested_loop_file, 1, Cmp_fct, Body_fct>;
  if (outer_estimator == NULL) {
    outer_estimator = &controller_type::controller.get_estimator();
  }
  
  pasl::pctl::granularity::cstmt(controller_type::controller, [&] { return comp; }, [&] {
    if (r - l == 0) {
      return;
    }
    if (r - l == 1) {
      body(l);
      return;
    }
    int mid = (l + r) >> 1;
    pasl::pctl::granularity::fork2([&] {
      parallel_for(l, mid, cmp_fct, body);
    }, [&] {     parallel_for(mid, r, cmp_fct, body);
    });
  }, [&] {
    for (int i = l; i < r; i++) {
      body(i);
    }
  });
}


void pbbs_pctl_call(pbbs::measured_type measured, std::pair<int, int>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  pasl::pctl::perworker::array<long, pasl::pctl::perworker::get_my_id> cnt;
  cnt.init(0);
  if (lib_type == "pbbs") {
    measured([&] {
      cilk_for (int i = 0; i < x.first; i++) {
        cilk_for (int j = 0; j < x.second; j++) {
          cnt.mine()++;
        }
      }
    });
  } else {
    measured([&] {
      parallel_for(0, x.first, [&] (int l, int r) { return (double)x.second * (r - l); }, [&] (int i) {
        parallel_for(0, x.second, [&] (int l, int r) { return r - l; }, [&] (int i) {
          cnt.mine()++;
        });

      });
    });
    printf("outer loop constant: %.3lf\n", outer_estimator->get_constant());
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default<std::string>("infile", "");
    if (infile != "") {
      deepsea::cmdline::dispatcher d;
      d.add("pair_int_int", [&] {
        std::pair<int, int> x = pasl::pctl::io::load<std::pair<int, int>>(infile);
        pbbs_pctl_call(measured, x);
      });
      d.dispatch("type");
      return;
    }
  });
  return 0;
}

/***********************************************************************/
