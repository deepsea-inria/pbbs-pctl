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
#include "refine.hpp"
#include "loaders.hpp"
#include "refine.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

pbbs::triangles<pbbs::_point2d<double>> to_pbbs(pasl::pctl::triangles<pasl::pctl::_point2d<double>>& x) {
  pbbs::triangles<pbbs::_point2d<double>> result;
  result.numPoints = x.num_points;
//  std::cerr << "!!! " << x.num_points << std::endl;
  result.P = (pbbs::_point2d<double>*) malloc(sizeof(pbbs::_point2d<double>) * x.num_points);
  for (int i = 0; i < x.num_points; i++) {
    result.P[i] = pbbs::_point2d<double>(x.p[i].x, x.p[i].y);
  }
  result.numTriangles = x.num_triangles;
//  std::cerr << "!!! " << x.num_triangles << std::endl;
  result.T = (pbbs::triangle*) malloc(sizeof(pbbs::triangle) * x.num_triangles);
  for (int i = 0; i < x.num_triangles; i++) {
    result.T[i] = pbbs::triangle(x.t[i].vertices[0], x.t[i].vertices[1], x.t[i].vertices[2]);
  }
  return result;
}

void pbbs_pctl_call(pbbs::measured_type measured, pasl::pctl::triangles<pasl::pctl::_point2d<double>>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    pbbs::triangles<pbbs::_point2d<double>> y = to_pbbs(x);
    measured([&] {
      pbbs::refine(y);
    });
  } else {
    measured([&] {
      pasl::pctl::refine(x);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      pasl::pctl::triangles<pasl::pctl::_point2d<double>> x = pasl::pctl::io::load<pasl::pctl::triangles<pasl::pctl::_point2d<double>>>(infile);
      pbbs_pctl_call(measured, x);
      return;
    }

//    }
  });
  return 0;
}

/***********************************************************************/
