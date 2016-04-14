#include "kdtree.hpp"
#include "prandgen.hpp"

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
  for (intT i = 0; i < n; i++) {
    pointT pl(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 0), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 1), 
		 p0.z);
    pointT pr(p0.x + shift.x * pasl::pctl::prandgen::hash<double>(4 * i + 2), 
		 p0.y + shift.y * pasl::pctl::prandgen::hash<double>(4 * i + 3), 
		 p1.z);
    result[i] = ray<point3d>(pl, pr - pl);
  }
}

} //end namespace
} //end namespace

#endif /*! _PBBS_PCTL_RAYS_GENERATOR_H_ !*/