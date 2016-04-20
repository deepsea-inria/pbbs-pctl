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

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = pasl::util::cmdline::parse_or_default_int("test", 0);
    int n = pasl::util::cmdline::parse_or_default_int("n", 10000000);
    bool files = pasl::util::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = pasl::util::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = pasl::util::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/geometryData/data/");
    system("mkdir tests");
    if (test == 0) {
      parray<point2d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point2d>(std::string("tests/random_in_cube_2d_txt_10000000"), path_to_data + std::string("2DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_2d(std::string("tests/random_in_cube_2d_") + std::to_string(n), n, false, false, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 1, point2d>(a, (int)a.size(), 1);
      });
    } else if (test == 1) {
      parray<point2d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point2d>(std::string("tests/random_plummer_2d_txt_10000000"), path_to_data + std::string("2Dkuzmin_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_plummer_2d(std::string("tests/random_plummer_2d_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 1, point2d>(a, (int)a.size(), 1);
      });
    } else if (test == 2) {
      parray<point3d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point3d>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_in_cube_3d_") + std::to_string(n), n, false, false, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 1, point3d>(a, (int)a.size(), 1);
      });
    } else if (test == 3) {
      parray<point3d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point3d>(std::string("tests/random_on_sphere_3d_txt_10000000"), path_to_data + std::string("3DonSphere_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_on_sphere_3d_") + std::to_string(n), n, false, true, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 1, point3d>(a, (int)a.size(), 1);
      });
    } else if (test == 4) {
      parray<point3d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point3d>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_uniform_3d(std::string("tests/random_in_cube_3d_") + std::to_string(n), n, false, true, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 10, point3d>(a, (int)a.size(), 10);
      });
    } else if (test == 5) {
      parray<point3d> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<point3d>(std::string("tests/random_in_cube_3d_txt_10000000"), path_to_data + std::string("3DinCube_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_points_plummer_3d(std::string("tests/random_plummer_3d_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::ANN<int, 10, point3d>(a, (int)a.size(), 10);
      });
    }
  });
  return 0;
}

/***********************************************************************/
