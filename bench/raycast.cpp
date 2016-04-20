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
#include "kdtree.hpp"
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
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/happy_ray_cast_dataset"), path_to_data + std::string("happyTriangles"), path_to_data + std::string("happyRays"), reload);
      triangles<point3d> tri(a.points.size(), a.triangles.size(), a.points.begin(), a.triangles.begin());
      measured([&] {
        pasl::pctl::kdtree::ray_cast(tri, a.rays.begin(), a.rays.size());
      });
    } else if (test == 1) {
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/angel_ray_cast_dataset"), path_to_data + std::string("angelTriangles"), path_to_data + std::string("angelRays"), reload);
      triangles<point3d> tri(a.points.size(), a.triangles.size(), a.points.begin(), a.triangles.begin());
      measured([&] {
        pasl::pctl::kdtree::ray_cast(tri, a.rays.begin(), a.rays.size());
      });
    } else if (test == 2) {
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/dragon_ray_cast_dataset"), path_to_data + std::string("dragonTriangles"), path_to_data + std::string("dragonRays"), reload);
      triangles<point3d> tri(a.points.size(), a.triangles.size(), a.points.begin(), a.triangles.begin());
      measured([&] {
        pasl::pctl::kdtree::ray_cast(tri, a.rays.begin(), a.rays.size());
      });
    }
  });
  return 0;
}

/***********************************************************************/
