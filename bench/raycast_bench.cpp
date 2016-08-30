/*!
 * \file raycast_bench.cpp
 * \brief Benchmarking script for parallel ray cast
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
#include "kdtree.hpp"
#include "loaders.hpp"
#include "kdTree.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

pbbs::_point3d<double> to_pbbs(pasl::pctl::_point3d<double> point) {
  return pbbs::_point3d<double>(point.x, point.y, point.z);
}

pbbs::_vect3d<double> to_pbbs(pasl::pctl::_vect3d<double> vect) {
  return pbbs::_vect3d<double>(vect.x, vect.y, vect.z);
}

pbbs::triangle to_pbbs(pasl::pctl::triangle t) {
  return pbbs::triangle(t.vertices[0], t.vertices[1], t.vertices[2]);
}

pbbs::ray<pbbs::_point3d<double>> to_pbbs(pasl::pctl::ray<pasl::pctl::_point3d<double>> ray) {
  return pbbs::ray<pbbs::_point3d<double>>(to_pbbs(ray.o), to_pbbs(ray.d));
}

template <class Item1, class Item2>
parray<Item1> to_pbbs(parray<Item2>& a) {
  parray<Item1> result(a.size());
  for (int i = 0; i < a.size(); i++) {
    result[i] = to_pbbs(a[i]);
  }
  return result;
}

void pbbs_pctl_call(pbbs::measured_type measured, pasl::pctl::io::ray_cast_test& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    parray<pbbs::_point3d<double>> points = to_pbbs<pbbs::_point3d<double>>(x.points);
    parray<pbbs::triangle> triangles = to_pbbs<pbbs::triangle>(x.triangles);
    parray<pbbs::ray<pbbs::_point3d<double>>> rays = to_pbbs<pbbs::ray<pbbs::_point3d<double>>>(x.rays);
    pbbs::triangles<pbbs::_point3d<double>> tri(points.size(), triangles.size(), points.begin(), triangles.begin());
    measured([&] {
      pbbs::rayCast(tri, rays.begin(), rays.size());
    });
  } else {  
    pasl::pctl::triangles<pasl::pctl::_point3d<double>> tri(x.points.size(), x.triangles.size(), x.points.begin(), x.triangles.begin());
    measured([&] {
      pasl::pctl::kdtree::ray_cast(tri, x.rays.begin(), x.rays.size());
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      std::string base;
      std::string extension;
      pasl::pctl::io::parse_filename(infile, base, extension);
      if (extension == "txt") {
        std::string infile2 = deepsea::cmdline::parse_string("infile2");
        pasl::pctl::io::ray_cast_test x = pasl::pctl::io::load<pasl::pctl::io::ray_cast_test>(infile, infile2);
        pbbs_pctl_call(measured, x);
      } else {
        pasl::pctl::io::ray_cast_test x = pasl::pctl::io::load<pasl::pctl::io::ray_cast_test>(infile);
        std::cerr << x.points.size() << " " << x.triangles.size() << " " << x.rays.size() << std::endl;
        pbbs_pctl_call(measured, x);
      }
      return;
    }
    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = deepsea::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/geometryData/data/");
    system("mkdir tests");
    if (test == 0) {
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/happy_ray_cast_dataset"), path_to_data + std::string("happyTriangles"), path_to_data + std::string("happyRays"), reload);
      pbbs_pctl_call(measured, a);
    } else if (test == 1) {
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/angel_ray_cast_dataset"), path_to_data + std::string("angelTriangles"), path_to_data + std::string("angelRays"), reload);
      pbbs_pctl_call(measured, a);
    } else if (test == 2) {
      pasl::pctl::io::ray_cast_test a = pasl::pctl::io::load_ray_cast_test(std::string("tests/dragon_ray_cast_dataset"), path_to_data + std::string("dragonTriangles"), path_to_data + std::string("dragonRays"), reload);
      pbbs_pctl_call(measured, a);
    }
  });
  return 0;
}

/***********************************************************************/
