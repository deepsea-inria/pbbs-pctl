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

#include <iostream>
#include <limits>
#include "octtree.hpp"
#include "parray.hpp"

#ifndef _PCTL_PBBS_NEAREST_NEIGHBORS_H_
#define _PCTL_PBBS_NEAREST_NEIGHBORS_H_

namespace pasl {
namespace pctl {
  
  using namespace std;
  
  // A k-nearest neighbor structure
  // requires vertexT to have pointT and vectT typedefs
  template <class intT, class vertexT, int maxK>
  struct nearest_neighbours_ds {
    typedef vertexT vertex;
    typedef typename vertexT::pointT point;
    typedef typename point::vectT fvect;
    
    typedef dimtree_node<intT, point, fvect, vertex> qotree;
    qotree *tree;
    
    // generates the search structure
    nearest_neighbours_ds(vertex** vertices, int n) {
      tree = qotree::dimtree(vertices, n);
    }
    
    // returns the vertices in the search structure, in an
    //  order that has spacial locality
    vertex** vertices() {
      return tree->flatten();
    }
    
    void del() {
      tree->del();
    }
    
    struct kNN {
      vertex* ps;  // the element for which we are trying to find a NN
      vertex* pn[maxK];  // the current k nearest neighbors (nearest last)
      double rn[maxK]; // radius of current k nearest neighbors
      int quads;
      int k;
      kNN() {}
      
      // returns the ith smallest element (0 is smallest) up to k-1
      vertex* operator[] (const int i) {
        return pn[k - i - 1];
      }
      
      kNN(vertex *p, int kk) {
        if (kk > maxK) {
          cout << "k too large in kNN" << endl;
          abort();
        }
        k = kk;
        quads = (1 << (p->pt).dimension());
        ps = p;
        for (int i=0; i < k; i++) {
          pn[i] = (vertex*) NULL;
          rn[i] = numeric_limits<double>::max();
        }
      }
      
      // if p is closer than pn then swap it in
      void update(vertex *p) {
        //inter++;
        point opt = (p->pt);
        fvect v = (ps->pt) - opt;
        double r = v.length();
        if (r < rn[0]) {
          pn[0] = p;
          rn[0] = r;
          for (int i = 1; i < k && rn[i - 1] < rn[i]; i++) {
            swap(rn[i - 1], rn[i]);
            swap(pn[i - 1], pn[i]);
          }
        }
      }
      
      // looks for nearest neighbors in boxes for which ps is not in
      void nearest_ngh_trim(qotree* tree) {
        if (!(tree->center).out_of_box(ps->pt, (tree->size / 2) + rn[0])) {
          if (tree->is_leaf()) {
            for (int i = 0; i < tree->count; i++) {
              update(tree->vertices[i]);
            }
          } else {
            for (int j = 0; j < quads; j++) {
              nearest_ngh_trim(tree->children[j]);
            }
          }
        }
      }
      
      // looks for nearest neighbors in box for which ps is in
      void nearest_ngh(qotree* tree) {
        if (tree->is_leaf()) {
          for (int i = 0; i < tree->count; i++) {
            vertex* pb = tree->vertices[i];
            if (pb != ps) {
              update(pb);
            }
          }
        } else {
          int i = tree->find_quadrant(ps);
          nearest_ngh(tree->children[i]);
          for (int j = 0; j < quads; j++) {
            if (j != i) {
              nearest_ngh_trim(tree->children[j]);
            }
          }
        }
      }
    };
    
    vertex* nearest(vertex* p) {
      kNN nn(p, 1);
      nn.nearest_ngh(tree);
      return nn[0];
    }
    
    // version that writes into result
    void nearest_k(vertex* p, vertex** result, int k) {
      kNN nn(p, k);
      nn.nearest_ngh(tree);
      for (int i = 0; i < k; i++) {
        result[i] = NULL;
      }
      for (int i = 0; i < k; i++) {
        result[i] = nn[i];
      }
    }
    
    // version that allocates result
    vertex** nearest_k(vertex *p, int k) {
      vertex** result = newA(vertex*, k);
      kNearest(p, result, k);
      return result;
    }
    
  };
  
  // find the k nearest neighbors for all points in tree
  // places pointers to them in the .ngh field of each vertex
  template <class intT, int maxK, class vertexT>
  void ANN(vertexT** v, int n, int k) {
    typedef nearest_neighbours_ds<intT, vertexT, maxK> kNNT;
    
    kNNT ds = kNNT(v, n);
    
    //cout << "built tree" << endl;
    
    // this reorders the vertices for locality
    vertexT** vr = ds.vertices();
    
    // find nearest k neighbors for each point
    parallel_for(int(0), n, [&] (int i) {
      ds.nearest_k(vr[i], vr[i]->ngh, k);
    });
    
    delete [] vr;
    ds.del();
  }
  
  template <class Point, int maxK>
  struct vertex {
    typedef Point pointT;

    int identifier;
    Point pt;         // the point itself
    vertex<Point, maxK>* ngh[maxK];    // the list of neighbors

    vertex(Point p, int id) : pt(p), identifier(id) {}

    vertex() {}
  };

  template <class intT, int maxK, class Point>
  parray<intT> ANN(parray<Point>& points, int n, int k) {
    parray<intT> result(n * k);

    parray<vertex<Point, maxK>*> vertices(points.size());
    parray<vertex<Point, maxK>> vv(points.size());
    parallel_for(0, (int)points.size(), [&] (int i) {
      vertices[i] = new (&vv[i]) vertex<Point, maxK>(points[i], i);
    });

    ANN<intT, maxK, vertex<Point, maxK>>(vertices.begin(), n, k);
    parallel_for(0, n, [&] (int i) {
      for (int j = 0; j < std::min(n - 1, k); j++) {
        result[i * maxK + j] = vertices[i]->ngh[j]->identifier;
      }
    });
    return result;
  }

} // end namespace
} // end namespace

#endif /*! _PCTL_PBBS_NEAREST_NEIGHBORS_H_ */
