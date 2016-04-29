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
};

}
}
}
#endif