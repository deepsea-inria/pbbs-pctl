#include "geometry.hpp"
#include "prandgen.hpp"
#include "teststructs.hpp"
#include "geometrydata.hpp"

#ifndef _PBBS_PCTL_RAYS_GENERATOR_H_
#define _PBBS_PCTL_RAYS_GENERATOR_H_

namespace pasl {
namespace pctl {

parray<ray<point3d>> generate_rays(int n, parray<point3d> points) {
  parray<ray<point3d>> result(n);

  point3d p0 = points[0];
  point3d p1 = points[0];
  for (int i = 0; i < n; i++) {
    p0 = p0.min_coord(points[i]);
    p1 = p1.max_coord(points[i]);
  }

  vect3d shift = p1 - p0;
  for (int i = 0; i < n; i++) {
    point3d pl(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 0), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 1), 
		 p0.z);
    point3d pr(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 2), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 3), 
		 p1.z);
    result[i] = ray<point3d>(pl, pr - pl);
  }
  return result;
}

io::ray_cast_test generate_ray_cast_test(int n, int on_sphere) {
  io::ray_cast_test test;
  test.points.resize(3 * n);
  test.triangles.resize(n);
  double d = 1.0 / sqrt((double) n);
  for (int i = 0; i < n; i++) {
    if (on_sphere) {
      test.points[3 * i] = randOnUnitSphere3d<int, unsigned int>((i + 1) * n);
    } else {
      test.points[3 * i] = rand3d<int, unsigned int>((i + 1) * n);
    }
    test.points[3 * i + 1] = test.points[3 * i] + vect3d(d, d, 0);
    test.points[3 * i + 2] = test.points[3 * i] + vect3d(d, 0, d);
    test.triangles[i].vertices[0] = 3 * i;
    test.triangles[i].vertices[1] = 3 * i + 1;
    test.triangles[i].vertices[2] = 3 * i + 2;
  }
  test.rays = generate_rays(n, test.points);
  return test;
}

} //end namespace
} //end namespace

#endif /*! _PBBS_PCTL_RAYS_GENERATOR_H_ !*/