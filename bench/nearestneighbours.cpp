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
#include "nearestneighbors.hpp"
#include "loaders.hpp"
#include "nearestNeighbors.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

parray<pbbs::_point2d<double>> to_pbbs(parray<_point2d<double>>& points) {
  parray<pbbs::_point2d<double>> result(points.size());
  for (int i = 0; i < points.size(); i++) {
    result[i] = pbbs::_point2d<double>(points[i].x, points[i].y);
  }
  return result;
}

parray<pbbs::_point3d<double>> to_pbbs(parray<_point3d<double>>& points) {
  parray<pbbs::_point3d<double>> result(points.size());
  for (int i = 0; i < points.size(); i++) {
    result[i] = pbbs::_point3d<double>(points[i].x, points[i].y, points[i].z);
  }
  return result;
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = deepsea::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/geometryData/data/");
    std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
    system("mkdir tests");
    if (test == 0) {
      parray<_point2d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point2d<double>>(std::string("tests/random_in_cube_2d_txt_10000000"), path_to_data + std::string("2DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_2d(std::string("tests/random_in_cube_2d_") + std::to_string(n), n, false, false, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point2d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<1, pbbs::_point2d<double>>(&b[0], (int)b.size(), 1);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 1, _point2d<double>>(a, (int)a.size(), 1);
        });
      }
    } else if (test == 1) {
      parray<_point2d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point2d<double>>(std::string("tests/random_plummer_2d_txt_10000000"), path_to_data + std::string("2Dkuzmin_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_plummer_2d(std::string("tests/random_plummer_2d_") + std::to_string(n), n, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point2d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<1, pbbs::_point2d<double>>(&b[0], (int)b.size(), 1);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 1, _point2d<double>>(a, (int)a.size(), 1);
        });
      }
    } else if (test == 2) {
      parray<_point3d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point3d<double>>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_in_cube_3d_") + std::to_string(n), n, false, false, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point3d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<1, pbbs::_point3d<double>>(&b[0], (int)b.size(), 1);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 1, _point3d<double>>(a, (int)a.size(), 1);
        });
      }
    } else if (test == 3) {
      parray<_point3d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point3d<double>>(std::string("tests/random_on_sphere_3d_txt_10000000"), path_to_data + std::string("3DonSphere_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_on_sphere_3d_") + std::to_string(n), n, false, true, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point3d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<1, pbbs::_point3d<double>>(&b[0], (int)b.size(), 1);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 1, _point3d<double>>(a, (int)a.size(), 1);
        });
      }
    } else if (test == 4) {
      parray<_point3d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point3d<double>>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_in_cube_3d_") + std::to_string(n), n, false, true, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point3d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<10, pbbs::_point3d<double>>(&b[0], (int)b.size(), 10);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 10, _point3d<double>>(a, (int)a.size(), 10);
        });
      }
    } else if (test == 5) {
      parray<_point3d<double>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<_point3d<double>>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_plummer_3d(std::string("tests/random_plummer_3d_") + std::to_string(n), n, reload);
      }
      if (lib_type == "pbbs") {
        parray<pbbs::_point3d<double>> b = to_pbbs(a);
        measured([&] {
          pbbs::findNearestNeighbors<10, pbbs::_point3d<double>>(&b[0], (int)b.size(), 10);
        });
      } else {
        measured([&] {
          pasl::pctl::ANN<int, 10, _point3d<double>>(a, (int)a.size(), 10);
        });
      }
    }
  });
  return 0;
}

/***********************************************************************/
