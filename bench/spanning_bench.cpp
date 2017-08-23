/*!
 * \file spanning_bench.cpp
 * \brief Benchmarking script for parallel spanning forest
 * \date 2017
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <math.h>
#include <functional>
#include <stdlib.h>
#include "bench.hpp"
#include "graphutils.hpp"
#include "spanning.hpp"
#include "loaders.hpp"
#include "spanning.h"

pbbs::graph::edgeArray<int> to_pbbs(pasl::pctl::graph::edgeArray<int>& g) {
  pbbs::graph::edge<int>* e = (pbbs::graph::edge<int>*) malloc(sizeof(pbbs::graph::edge<int>) * g.nonZeros);
  for (int i = 0; i < g.nonZeros; i++) {
    e[i] = pbbs::graph::edge<int>(g.E[i].u, g.E[i].v);
  }
  return pbbs::graph::edgeArray<int>(e, g.numRows, g.numCols, g.nonZeros);
}

void pbbs_pctl_call(pbbs::measured_type measured, pasl::pctl::graph::edgeArray<int>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    pbbs::graph::edgeArray<int> y = to_pbbs(x);
    measured([&] {
      pbbs::spanningTree(y);
    });
  } else {
    measured([&] {
      pasl::pctl::spanningTree(x);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      pasl::pctl::graph::graph<int> x = pasl::pctl::io::load<pasl::pctl::graph::graph<int>>(infile);
      pasl::pctl::graph::edgeArray<int> edges = to_edge_array(x);
      pbbs_pctl_call(measured, edges);
      return;
    }

  });
  return 0;
}
