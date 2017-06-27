#include "parray.hpp"

#ifndef _PCTL_TEST_STRUCTS_
#define _PCTL_TEST_STRUCTS_

namespace pasl {
namespace pctl {
namespace io {

class ray_cast_test {
public:
  pasl::pctl::parray<point3d> points;
  pasl::pctl::parray<triangle> triangles;
  pasl::pctl::parray<ray<point3d>> rays;

  ray_cast_test(): points(0), triangles(0), rays(0) { }

  ray_cast_test(const ray_cast_test& other) {
    points = other.points;
    triangles = other.triangles;
    rays = other.rays;
  }

  ray_cast_test& operator=(const ray_cast_test& other) {
    if (&other == this) {
      return *this;
    }
    this->points = other.points;
    this->triangles = other.triangles;
    this->rays = other.rays;
    ray_cast_test foo;
    return foo;
  }
};

}
}
}
#endif
