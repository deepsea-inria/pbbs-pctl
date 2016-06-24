#include "geometry.hpp"
#include "datapar.hpp"

namespace pasl {
  namespace pctl {
    using intT = int;
    typedef _point3d<double> pointT;
    
    intT* ray_cast(triangles<pointT>, ray<pointT>*, intT);
  }
}


