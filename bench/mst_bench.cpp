/*!
 * \file matching_bench.cpp
 * \brief Benchmarking script for parallel minimal spanning tree
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
#include "mst.hpp"
#include "loaders.hpp"
#include "mst.h"

pbbs::graph::wghEdgeArray<int> to_pbbs(pasl::pctl::graph::wghEdgeArray<int>& g) {
  pbbs::graph::wghEdge<int>* e = (pbbs::graph::wghEdge<int>*) malloc(sizeof(pbbs::graph::wghEdge<int>) * g.m);
  for (int i = 0; i < g.m; i++) {
    e[i] = pbbs::graph::wghEdge<int>(g.E[i].u, g.E[i].v, g.E[i].weight);
  }
  return pbbs::graph::wghEdgeArray<int>(e, g.n, g.m);
}

void pbbs_pctl_call(pbbs::measured_type measured, pasl::pctl::graph::wghEdgeArray<int>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    pbbs::graph::wghEdgeArray<int> y = to_pbbs(x);
    measured([&] {
      pbbs::mst(y);
    });
  } else {
    measured([&] {
      pasl::pctl::mst(x);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      pasl::pctl::graph::graph<int> x = pasl::pctl::io::load<pasl::pctl::graph::graph<int>>(infile);
      pasl::pctl::graph::wghEdgeArray<int> edges = to_weighted_edge_array(x);
      pbbs_pctl_call(measured, edges);
      return;
    }
  });
  return 0;
}
