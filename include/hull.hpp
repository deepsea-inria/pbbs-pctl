// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "dpsdatapar.hpp"
#include "geometry.hpp"

#ifndef _PCTL_PBBS_HULL_H_
#define _PCTL_PBBS_HULL_H_

namespace pasl {
namespace pctl {

using intT = int;
using uintT = unsigned int;

template <class Item>
using iter = typename parray<Item>::iterator;

template <class Item>
using citer = typename parray<Item>::const_iterator;

template <class F1, class F2>
pair<intT, intT> double_pack(iter<intT> a, intT n, F1 lf, F2 rf) {
  intT ll = 0, lm = 0;
  intT rm = n - 1, rr = n - 1;
  while (1) {
    while (lm <= rm && !rf(a[lm])) {
      if (lf(a[lm])) {
        a[ll++] = a[lm];
      }
      lm++;
    }
    while (rm >= lm && !lf(a[rm])) {
      if (rf(a[rm])) {
        a[rr--] = a[rm];
      }
      rm--;
    }
    if (lm >= rm) break;
    auto tmp = a[rm--];
    a[rr--] = a[lm++];
    a[ll++] = tmp;
  }
  intT n1 = ll;
  intT n2 = n - rr - 1;
  return pair<intT,intT>(n1, n2);
}

intT quick_hull_seq(iter<intT> indices, point2d* p, intT n, intT l, intT r) {
  if (n < 2) return n;
  intT split_point = indices[0];
  double max_area = triangle_area(p[l], p[r], p[split_point]);
  for (intT i = 1; i < n; i++) {
    intT j = indices[i];
    double a = triangle_area(p[l], p[r], p[j]);
    if (a > max_area) {
      max_area = a;
      split_point = j;
    }
  }
  
  pair<intT, intT> nn = double_pack(indices, n, [&] (intT i) {
    return triangle_area(p[l], p[split_point], p[i]) > EPS;
  }, [&] (intT i) {
    return triangle_area(p[split_point], p[r], p[i]) > EPS;
  });
  intT n1 = nn.first;
  intT n2 = nn.second;
  
  intT m1, m2;
  m1 = quick_hull_seq(indices, p, n1, l, split_point);
  m2 = quick_hull_seq(indices + n - n2, p, n2, split_point, r);
  for (intT i = 0; i < m2; i++) {
    indices[i + m1 + 1] = indices[i + n - n2];
  }
  indices[m1] = split_point;
  return m1 + 1 + m2;
}

controller_type quickhull_contr("quickhull");

intT quick_hull(iter<intT> indices, iter<intT> tmp, iter<point2d> p, intT n, intT l, intT r) {
  intT result;
  par::cstmt(quickhull_contr, [&] { return n; }, [&] {
    if (n < 2) {
      result = quick_hull_seq(indices, p, n, l, r);
    } else {
      auto greater = [&] (double x, double y) {
        return x > y;
      };

      // Finding some point on the convex hull between l and r
/*      intT idx = (intT)max_index(indices, indices + n, (double)0.0, greater, [&] (intT j, double) {
          std::cerr << j << std::endl;
        return triangle_area(p[l], p[r], p[indices[j]]);
      });
      intT split = indices[idx];*/
      intT split = (intT)reduce(indices, indices + n, indices[0], [&] (intT x, intT y) {
        if (triangle_area(p[l], p[r], p[x]) > triangle_area(p[l], p[r], p[y])) {
          return x;
        } else {
          return y;
        }
      });

/*      if (split_point != split) {
        std::cerr << "Fuck off! " << max_area << " " << triangle_area(p[l], p[r], p[split]) << " " << split_point << " " << split << " " << n << std::endl;
      }*/

      
      // vertices on the convex hull between l and split
      intT n1 = (intT)dps::filter(indices, indices + n, tmp, [&] (intT i) {
        return triangle_area(p[l], p[split], p[i]) > EPS;
      });
      // vertices on the convex hull between split and r
      intT n2 = (intT)dps::filter(indices, indices + n, tmp + n1, [&] (intT i) {
        return triangle_area(p[split], p[r], p[i]) > EPS;
      });
      
      intT m1, m2;
      par::fork2([&] {
        m1 = quick_hull(tmp, indices, p, n1, l, split);
      }, [&] {
        m2 = quick_hull(tmp + n1, indices + n1, p, n2, split, r);
      });
      
      pmem::copy(tmp, tmp + m1, indices);
      indices[m1] = split;
      pmem::copy(tmp + n1, tmp + n1 + m2, indices + m1 + 1);
      result = m1 + 1 + m2;
    }
  }, [&] {
    result = quick_hull_seq(indices, p, n, l, r);
  }); 
  return result;
}

parray<intT> hull(parray<point2d>& p) {
  intT n = (intT)p.size();
  auto combine = [&] (pair<intT, intT> l, pair<intT, intT> r) {
    intT min_index =
    (p[l.first].x < p[r.first].x) ? l.first :
    (p[l.first].x > p[r.first].x) ? r.first :
    (p[l.first].y < p[r.first].y) ? l.first : r.first;
    intT max_index = (p[l.second].x > p[r.second].x) ? l.second : r.second;
    return make_pair(min_index, max_index);
  };
  auto id = make_pair(0, 0);
  auto min_max = level1::reducei(p.cbegin(), p.cend(), id, combine, [&] (long i, point2d) {
    return make_pair(i, i);
  });
  intT l = min_max.first;
  intT r = min_max.second;
  parray<bool> is_top_hull(n);
  parray<bool> is_bottom_hull(n);
  parray<intT> indices(n);
  parray<intT> tmp(n);
  parallel_for((intT)0, n, [&] (intT i) {
    tmp[i] = i;
    double a = triangle_area(p[l], p[r], p[i]);
    is_top_hull[i] = a > EPS;
    is_bottom_hull[i] = a < -EPS;
  });
  
  intT n1 = (intT)dps::pack(is_top_hull.cbegin(), tmp.cbegin(), tmp.cend(), indices.begin());
  intT n2 = (intT)dps::pack(is_bottom_hull.cbegin(), tmp.cbegin(), tmp.cend(), indices.begin() + n1);
/*  cout << n1 << " " << n2 << " " << l << " " << r << endl;
  if (n2 == 2) {
    for (int i = 0; i < n1; i++) {
      cout << indices[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < n2; i++) {
      cout << indices[i + n1] << " ";
    }
    cout << endl;
  }*/

  intT m1; intT m2;
  par::fork2([&] {
    m1 = quick_hull(indices.begin(), tmp.begin(), p.begin(), n1, l, r);
//    m1 = quick_hull_seq(indices.begin(), p.begin(), n1, l, r);
  }, [&] {
    m2 = quick_hull(indices.begin() + n1, tmp.begin() + n1, p.begin(), n2, r, l);
//    m2 = quick_hull_seq(indices.begin() + n1, p.begin(), n2, r, l);
  });
  
  pmem::copy(indices.cbegin(), indices.cbegin() + m1, tmp.begin() + 1);
  pmem::copy(indices.cbegin() + n1, indices.cbegin() + n1 + m2, tmp.begin() + m1 + 2);
  
  tmp[0] = l;
  tmp[m1 + 1] = r;
  tmp.resize(m1 + 2 + m2);
  return tmp;
}

} // end namespace
} // end namespace

#endif /*! _PCTL_PBBS_HULL_H_ */