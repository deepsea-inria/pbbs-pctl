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

#include <vector>
#include "datapar.hpp"
#include "geometry.hpp"
#include "nearestneighbors.hpp"
#include "topology.hpp"

#ifndef _PCTL_DELAUNAY_TRI_H_
#define _PCTL_DELAUNAY_TRI_H_

namespace pasl {
namespace pctl {
    
using namespace std;

// if on verifies the Delaunay is correct
#define CHECK 0

// *************************************************************
//    ROUTINES FOR FINDING AND INSERTING A NEW POINT
// *************************************************************

// Finds a vertex (p) in a mesh starting at any triangle (start)
// Requires that the mesh is properly connected and convex
simplex find(vertex *p, simplex start) {
  simplex t = start;
  while (1) {
    int i;
    for (i = 0; i < 3; i++) {
      t = t.rotClockwise();
      if (t.outside(p)) {t = t.across(); break;}
    }
    if (i == 3) return t;
    if (!t.valid()) return t;
  }
}

// Holds vertex and simplex queues used to store the cavity created
// while searching from a vertex between when it is initially searched
// and later checked to see if all corners are reserved.
struct queues {
  vector<vertex*> vertex_queue;
  vector<simplex> simplex_queue;
};

// Recursive routine for finding a cavity across an edge with
// respect to a vertex p.
// The simplex has orientation facing the direction it is entered.
//
//         a
//         | \ --> recursive call
//   p --> |T c
// enter   | / --> recursive call
//         b
//
//  If p is in circumcircle of T then
//     add T to simplex_queue, c to vertex_queue, and recurse
void findCavity(simplex t, vertex *p, queues *q) {
  if (t.inCirc(p)) {
    q->simplex_queue.push_back(t);
    t = t.rotClockwise();
    findCavity(t.across(), p, q);
    q->vertex_queue.push_back(t.firstVertex());
    t = t.rotClockwise();
    findCavity(t.across(), p, q);
  }
}

// Finds the cavity for v and tries to reserve vertices on the
// boundary (v must be inside of the simplex t)
// The boundary vertices are pushed onto q->vertex_queue and
// simplices to be deleted on q->simplex_queue (both initially empty)
// It makes no side effects to the mesh other than to X->reserve
void reserve_for_insert(vertex* v, simplex t, queues* q) {
  // each iteration searches out from one edge of the triangle
  for (int i = 0; i < 3; i++) {
    q->vertex_queue.push_back(t.firstVertex());
    findCavity(t.across(), v, q);
    t = t.rotClockwise();
  }
  // the maximum id new vertex that tries to reserve a boundary vertex
  // will have its id written.  reserve starts out as -1
  for (intT i = 0; i < q->vertex_queue.size(); i++)
    utils::writeMax(&((q->vertex_queue)[i]->reserve), v->id);
}

// checks if v "won" on all adjacent vertices and inserts point if so
bool insert(vertex *v, simplex t, queues *q) {
  bool flag = 0;
  for (intT i = 0; i < q->vertex_queue.size(); i++) {
    vertex* u = (q->vertex_queue)[i];
    if (u->reserve == v->id) u->reserve = -1; // reset to -1
    else flag = 1; // someone else with higher priority reserved u
  }
  if (!flag) {
    tri* t1 = v->t;  // the memory for the two new triangles
    tri* t2 = t1 + 1;
    // the following 3 lines do all the side effects to the mesh.
    t.split(v, t1, t2);
    for (intT i = 0; i < q->simplex_queue.size(); i++) {
      (q->simplex_queue)[i].flip();
    }
  }
  q->simplex_queue.clear();
  q->vertex_queue.clear();
  return flag;
}

// *************************************************************
//    CHECKING THE TRIANGULATION
// *************************************************************

void checkDelaunay1(tri *triangs, intT n, intT boundarySize) {
  parray<intT> bcount(n, 0);
  parallel_for((intT)0, n, [&] (intT i) {
    if (triangs[i].initialized >= 0) {
      simplex t = simplex(&triangs[i],0);
      for (int i=0; i < 3; i++) {
        simplex a = t.across();
        if (a.valid()) {
          vertex* v = a.rotClockwise().firstVertex();
          if (!t.outside(v)) {
            cout << "Inside Out: "; v->pt.print(); t.print();}
          if (t.inCirc(v)) {
            cout << "In Circle Violation: "; v->pt.print(); t.print(); }
        } else bcount[i]++;
        t = t.rotClockwise();
      }
    }
  });
  if (boundarySize != sum(bcount.cbegin(), bcount.cend()))
    cout << "Wrong boundary size: should be " << boundarySize
    << " is " << bcount << endl;
}

// *************************************************************
//    CREATING A BOUNDING CIRCULAR REGION AND FILL WITH INITIAL SIMPLICES
// *************************************************************

struct minpt {
  point2d operator() (point2d a, point2d b) {return a.min_coord(b);}};
struct maxpt {
  point2d operator() (point2d a, point2d b) {return a.max_coord(b);}};

// P is the set of points to bound and n the number
// bCount is the number of points to put on the boundary
// v is an array to put the new points
// t is an array to put the new triangles
simplex generate_boundary(point2d* p, intT n, intT boundary_size, vertex* v, tri* t) {
  point2d minP = reduce(p, p + n, p[0], [&] (point2d a, point2d b) {
    return a.min_coord(b);
  });
  point2d maxP = reduce(p, p + n, p[0], [&] (point2d a, point2d b) {
    return a.max_coord(b);
  });
  double size = (maxP - minP).length();
  double stretch = 10.0;
  double radius = stretch * size;
  point2d center = maxP + (maxP - minP)/2.0;
  
  point2d* boundary = newA(point2d, boundary_size);
  double pi = 3.14159;
  
  // Generate the bounding points on a circle far outside the bounding box
  for (intT i=0; i < boundary_size; i++) {
    double x = radius * cos(2 * pi * ((float) i)/((float) boundary_size));
    double y = radius * sin(2 * pi * ((float) i)/((float) boundary_size));
    boundary[i] = center + vect2d(x, y);
    v[i] = vertex(boundary[i], i + n);
  }
  
  // Fill with simplices (bCount - 2  total simplices)
  simplex s = simplex(&v[0], &v[1], &v[2], t);
  for (intT i = 3; i < boundary_size; i++)
    s = s.extend(&v[i], t + i - 2);
  return s;
}


// *************************************************************
//    MAIN LOOP
// *************************************************************

void incrementally_add_points(vertex** v, intT n, vertex* start) {
  
  // various structures needed for each parallel insertion
  intT maxR = (intT) (n / 100) + 1; // maximum number to try in parallel
  parray<queues> qqs;
  qqs.prefix_tabulate(maxR, 0);
  parray<queues*> qs;
  qs.prefix_tabulate(maxR, 0);
  for (intT i = 0; i < maxR; i++) {
    qs[i] = new (&qqs[i]) queues;
  }
  parray<simplex> t(maxR);
  parray<bool> flags(maxR);
  parray<vertex*> h(maxR);
  
  // create a point location structure
  typedef nearest_neighbours_ds<intT, vertex, 1> KNN;
  KNN knn = KNN(&start, 1);
  int multiplier = 8; // when to regenerate
  intT nextNN = multiplier;
  
  intT top = n; intT rounds = 0; intT failed = 0;
  
  // process all vertices starting just below top
  while (top > 0) {
    
    // every once in a while create a new point location
    // structure using all points inserted so far
    if ((n - top) >= nextNN && (n - top) < n / multiplier) {
      knn.del();
      knn = KNN(v + top, n - top);
      nextNN = nextNN * multiplier;
    }
    
    // determine how many vertices to try in parallel
    intT cnt = 1 + (n - top) / 100;  // 100 is pulled out of a hat
    cnt = (cnt > maxR) ? maxR : cnt;
    cnt = (cnt > top) ? top : cnt;
    vertex** vv = v + top - cnt;
    
    // for trial vertices find containing triangle, determine cavity
    // and reserve vertices on boundary of cavity
    parallel_for((intT)0, cnt, [&] (intT j) {
      vertex *u = knn.nearest(vv[j]);
      t[j] = find(vv[j], simplex(u->t, 0));
      reserve_for_insert(vv[j], t[j], qs[j]);
    });
    
    // For trial vertices check if they own their boundary and
    // update mesh if so.  flags[i] is 1 if failed (need to retry)
    parallel_for((intT)0, cnt, [&] (intT j) {
      flags[j] = insert(vv[j], t[j], qs[j]);
    });
    
    // Pack failed vertices back onto Q and successful
    // ones up above (needed for point location structure)
    intT k = (intT)dps::pack(flags.cbegin(), vv, vv + cnt, h.begin());
//    intT k = sequence::pack(vv,h,flags,cnt);
    parallel_for((intT)0, cnt, [&] (intT j) {
      flags[j] = !flags[j];
    });
    dps::pack(flags.cbegin(), vv, vv + cnt, h.begin() + k);
//    sequence::pack(vv,h+k,flags,cnt);
    parallel_for((intT)0, cnt, [&] (intT j) {
      vv[j] = h[j];
    });
    
    failed += k;
    top = top - cnt + k; // adjust top, accounting for failed vertices
    rounds++;
  }
  
  knn.del();
  
  //cout << "n=" << n << "  Total retries=" << failed
  //     << "  Total rounds=" << rounds << endl;
}


// *************************************************************
//    DRIVER
// *************************************************************

// A structure for generating a pseudorandom permutation
struct hashID {
  
private:
  long k;
  intT n;
  intT GCD(intT a, intT b) {
    while( 1 ) {
      a = a % b;
      if( a == 0 ) return b;
      b = b % a;
      if( b == 0 ) return a;
    }
  }
  
public:
  hashID(intT nn) : n(nn) {
    k = prandgen::hashi(nn)%nn;
    while (GCD(k,nn) > 1) k = (k + 1) % nn;
  }
  intT get(intT i) { return (i*k)%n; }
};

triangles<point2d> delaunay(parray<point2d>& p) {
  intT boundary_size = 10;
  int n = p.size();
  
  // allocate space for vertices
  intT num_vertices = n + boundary_size;
  parray<vertex*> v(n); // don't need pointers to boundary
  v.prefix_tabulate(n, 0);
  parray<vertex> vv;
  vv.prefix_tabulate(num_vertices, 0);
  
  // The points are psuedorandomly permuted
  
  hashID hash(n);
  parallel_for((intT)0, n, [&] (intT i) {
    v[i] = new (&vv[i]) vertex(p[hash.get(i)], i);
  });
  
  // allocate all the triangles needed
  intT num_triangles = 2 * n + (boundary_size - 2);
  parray<tri> triangles;
  triangles.prefix_tabulate(num_triangles, 0);
  
  // give two triangles to each vertex
  parallel_for((intT)0, n, [&] (intT i) {
    v[i]->t = triangles.begin() + 2 * i;
  });
  
  // generate boundary points and fill with simplices
  // The boundary points and simplices go at the end
  simplex sboundary = generate_boundary(p.begin(), n, boundary_size, vv.begin() + n, triangles.begin() + 2*n);
  vertex* v0 = sboundary.t->vtx[0];
  
  // main loop to add all points
  incrementally_add_points(v.begin(), n, v0);
  v.clear();
  if (CHECK) checkDelaunay1(triangles.begin(), num_triangles, boundary_size);
  
  triangle* rt = newA(triangle, num_triangles);
  
  // Since points were permuted need to translate back to
  // original coordinates
  parray<intT> m(num_vertices, [&] (intT i) {
    if (i < n) {
      return hash.get(i);
    } else {
      return i;
    }
  });
  
  parallel_for((intT)0, num_triangles, [&] (intT i) {
    vertex** vtx = triangles[i].vtx;
    rt[i] = triangle(m[vtx[0]->id], m[vtx[1]->id], m[vtx[2]->id]);
  });
  m.clear();
  point2d* rp = newA(point2d, num_vertices);
  parallel_for((intT)0, n, [&] (intT i) {
    rp[i] = p[i];
  });
  parallel_for((intT)n, num_vertices, [&] (intT i) {
    rp[i] = vv[i].pt;
  });
  
  return pasl::pctl::triangles<point2d>(num_vertices, num_triangles, rp, rt);
}
  
// Note that this is not currently a complete test of correctness
// For example it would allow a set of disconnected triangles, or even no
// triangles
bool checkDelaunay(tri *triangs, intT n, intT boundarySize) {
  parray<intT> bcount(n, 0);
  intT insideOutError = n;
  intT inCircleError = n;
  parallel_for((intT)0, n, [&] (intT i) {
    if (triangs[i].initialized) {
      simplex t = simplex(&triangs[i],0);
      for (int j=0; j < 3; j++) {
        simplex a = t.across();
        if (a.valid()) {
          vertex* v = a.rotClockwise().firstVertex();
          
          // Check that the neighbor is outside the triangle
          if (!t.outside(v)) {
            double vz = triAreaNormalized(t.t->vtx[(t.o+2)%3]->pt,
                                          v->pt, t.t->vtx[t.o]->pt);
            //cout << "i=" << i << " vz=" << vz << endl;
            // allow for small error
            if (vz < -1e-10)  utils::writeMin(&insideOutError,i);
          }
          
          // Check that the neighbor is not in circumcircle of the triangle
          if (t.inCirc(v)) {
            double vz = inCircleNormalized(t.t->vtx[0]->pt, t.t->vtx[1]->pt,
                                           t.t->vtx[2]->pt, v->pt);
            //cout << "i=" << i << " vz=" << vz << endl;
            // allow for small error
            if (vz > 1e-10) utils::writeMin(&inCircleError,i);
          }
        } else bcount[i]++;
        t = t.rotClockwise();
      }
    }
  });
  //intT cnt = sum(bcount.cbegin(), bcount.cend());
  //if (boundarySize != cnt) {
  //cout << "delaunayCheck: wrong boundary size, should be " << boundarySize
  //<< " is " << cnt << endl;
  //return 1;
  //}
  
  if (insideOutError < n) {
    cout << "delaunayCheck: neighbor inside triangle at triangle "
    << inCircleError << endl;
    return 1;
  }
  if (inCircleError < n) {
    cout << "In Circle Violation at triangle " << inCircleError << endl;
    return 1;
  }
  
  return 0;
}
  
} // end namespace
} // end namespace

#endif /*! _PCTL_DELAUNAY_TRI_H_ */
