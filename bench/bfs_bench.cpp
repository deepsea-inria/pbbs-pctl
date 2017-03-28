/*!
 * \file bfs_bench.cpp
 * \brief Benchmarking script for parallel bread-first search
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
#include "bfs.hpp"
#include "loaders.hpp"
#include "bfs.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

pbbs::graph::graph<int> to_pbbs(pasl::pctl::graph::graph<int>& g) {
  pbbs::graph::vertex<int>* v = (pbbs::graph::vertex<int>*) malloc(sizeof(pbbs::graph::vertex<int>) * g.n);
  for (int i = 0; i < g.n; i++) {
    v[i] = pbbs::graph::vertex<int>(g.V[i].Neighbors, g.V[i].degree);
  }
  return pbbs::graph::graph<int>(v, g.n, g.m, g.allocatedInplace);
}

void pbbs_pctl_call(pbbs::measured_type measured, pasl::pctl::graph::graph<int>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  int source = deepsea::cmdline::parse_or_default_int("source", 0);
  if (lib_type == "pbbs") {
    pbbs::graph::graph<int> y = to_pbbs(x);
    measured([&] {
      pbbs::BFS(source, y);
    });
  } else {
    measured([&] {
      pasl::pctl::bfs(source, x);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      pasl::pctl::graph::graph<int> x = pasl::pctl::io::load<pasl::pctl::graph::graph<int>>(infile);
      pbbs_pctl_call(measured, x);
      return;
    }

  });
  return 0;
}

/***********************************************************************/
