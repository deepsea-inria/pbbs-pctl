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
#include <cstdlib>
#include "geometry.hpp"
#include "blockradixsort.hpp"

#ifndef _PCTL_BENCH_OCTTREE_INCLUDED
#define _PCTL_BENCH_OCTTREE_INCLUDED

namespace pasl {
namespace pctl {
  
  using namespace std;
  
  // *************************************************************
  //    QUAD/OCT TREE NODES
  // *************************************************************
constexpr char octtree_file[] = "octtree";
#define max_leaf_size 16  // number of points stored in each leaf
#define GTREE_BASE_CASE 65536
#define GTREE_SUBPROB_POW .92
  
  template <class intT, class vertex, class point>
  struct ndata {
    intT cnt;
    ndata(intT x) : cnt(x) {}
    ndata(point center) : cnt(0) {}
    ndata& operator+=(const ndata op) {
      cnt += op.cnt;
      return *this;
    }
    ndata& operator+=(const vertex* op) {
      cnt += 1;
      return *this;
    }
  };
  
  template <class intT, class pointT, class vectT, class vertexT, class node_data = ndata<intT, vertexT, pointT> >
  class dimtree_node {
    private :
    
    public :
    typedef pointT point;
    typedef vectT fvect;
    typedef vertexT vertex;
    
    point center; // center of the box
    double size;   // width of each dimension, which have to be the same
    node_data data; // total mass of vertices in the box
    intT count;  // number of vertices in the box
    dimtree_node* children[8];
    vertex** vertices;
    bool crazy_leaf;
    
    // wraps a bounding box around the points and generates
    // a tree.
    static dimtree_node* dimtree(vertex** vv, intT n) {
      
      // calculate bounding box
      
      // copying to an array of points to make reduce more efficient
      parray<point> pt(n, [&] (intT i) {
        return vv[i]->pt;
      });

      assert(n >= 1);  // later: relax this constraint?
      point id = pt[0];
      point minPt = reduce(pt.cbegin(), pt.cend(), id, [&] (point a, point b) {
        return a.min_coord(b);
      });
      point maxPt = reduce(pt.cbegin(), pt.cend(), id, [&] (point a, point b) {
        return a.max_coord(b);
      });

      //cout << "min "; minPt.print(); cout << endl;
      //cout << "max "; maxPt.print(); cout << endl;
      fvect box = maxPt - minPt;
      point center = minPt + (box / 2.0);
      
      // copy before calling recursive routine since recursive routine is destructive
      parray<vertex*> v(vv, vv + n);
      //cout << "about to build tree" << endl;
      dimtree_node* result = new dimtree_node(v.begin(), n, center, box.max_dim());

      return result;
    }
    
    int is_leaf() {
      return (vertices != NULL);
    }
    
    void del() {
      if (is_leaf() && !crazy_leaf) {
        delete [] vertices;
      } else {
        for (int i = 0 ; i < (1 << center.dimension()); i++) {
          children[i]->del();
          delete children[i];
        }
      }
    }
    
    // Returns the depth of the tree rooted at this node
    intT depth() {
      intT res;
      if (is_leaf()) {
       res = 0;
      } else {
        res = 0;
        for (intT i = 0; i < (1 << center.dimension()); i++) {
          res = max(res, children[i]->depth());
        }
      }
      return res + 1;
    }
    
    // Returns the size of the tree rooted at this node
    intT points_count() {
      intT sz;
      if (is_leaf()) {
        sz = count;
        for (intT i = 0; i < count; i++) {
          if (vertices[i] < ((vertex*) NULL) + 1000)
            cout << "oops: " << vertices[i] << "," << count << "," << i << endl;
        }
      } else {
        sz = 0;
        for (int i = 0; i < (1 << center.dimension()); i++) {
          sz += children[i]->points_count();
        }
      }
      return sz;
    }
    
    template <class F>
    void apply_index(intT s, F f) {
      if (is_leaf()) {
        for (intT i = 0; i < count; i++) {
          f(vertices[i], s + i);
        }
      } else {
        intT ss = s;
        int nb = (1 << center.dimension());
        parray<intT> pss(nb);
        for (int i = 0; i < nb; i++) {
          pss[i] = ss;
          ss += children[i]->count;
        }
        range::parallel_for(int(0), nb, [&] (int l, int r) {
          return (r == nb ? ss : pss[r]) - pss[l];
        }, [&] (int i) {
          children[i]->apply_index(pss[i], f);
        });
      }
    }
    
    vertex** flatten() {
      vertex** a = new vertex*[count];
      apply_index(0, [&] (vertex* v, intT ind) {
        a[ind] = v;
      });
      return a;
    }
    
    // Returns the child the vertex p appears in
    int find_quadrant(vertex* p) {
      return (p->pt).quadrant(center);
    }

    int find_block(point2d min_pt, double blocksize, int log_numblocks, point2d p) {
      int numblocks = (1 << log_numblocks);
      int xindex = std::min((int)((p.x - min_pt.x) / blocksize), numblocks - 1);  
      int yindex = std::min((int)((p.y - min_pt.y) / blocksize), numblocks - 1);  
      int result = 0;
      for (int i = 0; i < log_numblocks; i++) {
        int mask = (1 << i);
        result += (xindex & mask) << i;
        result += (yindex & mask) << (i + 1);
      }
      return result;
    }

    int find_block(point3d min_pt, double blocksize, int log_numblocks, point3d p) {
      int numblocks = (1 << log_numblocks);
      int xindex = std::min((int)((p.x - min_pt.x) / blocksize), numblocks - 1);  
      int yindex = std::min((int)((p.y - min_pt.y) / blocksize), numblocks - 1);  
      int zindex = std::min((int)((p.z - min_pt.z) / blocksize), numblocks - 1);  
      int result = 0;
      for (int i = 0; i < log_numblocks; i++) {
        int mask = (1 << i);
        result += (xindex & mask) << (2 * i);
        result += (yindex & mask) << (2 * i + 1);
        result += (zindex & mask) << (2 * i + 2);
      }
      return result;
    }
    
    // A hack to get around Cilk shortcomings
    static dimtree_node *new_tree(vertex** v, intT n, point cnt, double sz) {
      return new dimtree_node(v, n, cnt, sz);
    }
    
    // Used as a closure in collectD
    struct findChild {
      dimtree_node* tr;
      findChild(dimtree_node *t) : tr(t) {}
      int operator() (vertex* p) {
        int r = tr->find_quadrant(p);
        return r;
      }
    };

    dimtree_node(point cnt, double sz) : data(node_data(cnt)) {
      count = 0;
      size = sz;
      center = cnt;
      vertices = NULL;
      crazy_leaf = false;
    }

    void build_recursive_tree(vertex** v, int n, int* offsets, int quadrants, dimtree_node* parent, int nodes_to_left, int height, int depth) {
      parent->count = 0;

      if (height == 1) {
        auto comp_fct = [&] (int l, int r) {
          return (r == quadrants ? n : offsets[r]) - offsets[l];
        };
        range::parallel_for(0, (int)(1 << center.dimension()), comp_fct, [&] (int i) {
           point new_center = (parent->center).offset_point(i, parent->size / 4.0);
           int q = (nodes_to_left << center.dimension()) + i;
           int l = ((q == quadrants - 1) ? n : offsets[q + 1]) - offsets[q];
           parent->children[i] = new_tree(v + offsets[q], l, new_center, parent->size / 2.0);
        });
      } else {
        auto comp_fct = [&] (int l, int r) {
          int ll = l << (center.dimension() * (height - 1));
          int rr = (r + 1) << (center.dimension() * (height - 1)) - 1;
          return (rr >= quadrants ? n : offsets[rr]) - offsets[ll];
        };
        range::parallel_for(0, (int)(1 << center.dimension()), comp_fct, [&] (int i) {
          point new_center = (parent->center).offset_point(i, parent->size / 4.0);
          parent->children[i] = new dimtree_node(new_center, parent->size / 2.0);
          build_recursive_tree(v, n, offsets, quadrants, parent->children[i], (nodes_to_left << center.dimension()) + i, height - 1, depth + 1);
        });
      }
      for (int i = 0; i < 1 << center.dimension(); i++) {
        parent->data += (parent->children[i])->data;
        parent->count += (parent->children[i])->count;
      }
      if (parent->count == 0) {
        parent->vertices = v;
        //TODO: to think about
        parent->crazy_leaf = true;
 //       parent->count = n;
      }
    }
    
    void sort_blocks_big(vertex** v, int cnt, int quadrants, int logdivs, double size, point center, int* offsets) {
      std::pair<int, vertex*>* blk = new std::pair<int, vertex*>[cnt];
      double blocksize = size / (double)(1 << logdivs);
      point min_pt = center.offset_point(0, size / 2);
      parallel_for(0, (int)cnt, [&] (int i) {
        blk[i] = make_pair(find_block(min_pt, blocksize, logdivs, v[i]->pt), v[i]);
      });
      intsort::integer_sort(blk, offsets, cnt, quadrants, [&] (std::pair<int, vertex*>& x) { return x.first; });
      parallel_for(0, (int)cnt, [&] (int i) {
        v[i] = blk[i].second;
      });
      delete [] blk;
    }

    // the recursive routine for generating the tree -- actually mutually recursive
    // due to newTree
    dimtree_node(vertex** v, intT n, point cnt, double sz) : data(node_data(cnt)) {
      //cout << "n=" << n << endl;
      count = n;
      size = sz;
      center = cnt;
      vertices = NULL;
      crazy_leaf = false;
      int quadrants = (1 << center.dimension());
      
      int logdivs = (int)(log2(count) * GTREE_SUBPROB_POW / (double)center.dimension());
      auto seq = [&] {
        if (count > max_leaf_size) {
          intT offsets[8];
          dimtree_node* now = this;
          intsort::integer_sort(v, offsets, n, (intT)quadrants, [&] (vertex* vertex) {
            return now->find_quadrant(vertex);
          });
          if (0) {
            for (intT i = 0; i < n; i++) {
              cout << "  " << i << ":" << this->find_quadrant(v[i]);
            }
          }
          //for (int i=0; i < quadrants; i++)
          //cout << i << ":" << offsets[i] << "  ";
          //cout << endl;
          // Give each child its appropriate center and size
          // The centers are offset by size/4 in each of the dimensions
          auto comp_fct = [&] (int l, int r) {
            return (r == quadrants ? n : offsets[r]) - offsets[l];
          };
//          range::parallel_for(int(0), quadrants, comp_fct, [&] (int i) {
          for (int i = 0; i < quadrants; i++) {
            point newcenter = center.offset_point(i, size / 4.0);
            intT l = ((i == quadrants - 1) ? n : offsets[i + 1]) - offsets[i];
            children[i] = new_tree(v + offsets[i], l, newcenter, size / 2.0);
          }
//          });
          
          data = node_data(center);
          for (int i = 0; i < quadrants; i++) {
            if (children[i]->count > 0) {
              data += children[i]->data;
            }
          }
        } else {
          vertices = new vertex*[count];
          data = node_data(center);
          for (intT i = 0; i < count; i++) {
            vertex* p = v[i];
            data += p;
            vertices[i] = p;
          }
        }
      };
#ifdef MANUAL_CONTROL
      if (logdivs == 1 || count <= GTREE_BASE_CASE) {
        seq();
        return;
      }
#endif
      if (count < max_leaf_size) {
        seq();
        return;
      }
      using controller_type = pasl::pctl::granularity::controller_holder<octtree_file, 1, vertex>;
      pasl::pctl::granularity::cstmt(controller_type::controller, [&] { return n; }, [&] {
        int divisions = (1 << logdivs); // number of quadrants in each dimension
        int quadrants = (1 << (center.dimension() * logdivs)); // total number
        int* offsets = new int[quadrants];
        sort_blocks_big(v, count, quadrants, logdivs, this->size, center, offsets);
        build_recursive_tree(v, n, offsets, quadrants, this, 0, logdivs, 1);
        delete [] offsets;
      }, seq);
    }
  };
  
} // end namespace
} // end namespace

#endif // _PCTL_BENCH_OCTTREE_INCLUDED
