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
#include <chrono>
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
#define DIMTREE_BASE_CASE 65536
#define DIMTREE_SUBPROB_POW .92
#define DIMTREE_ALLOC_FACTOR 2/max_leaf_size  

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
    
    dimtree_node* node_memory;

//    static parray<vertex*> wv;
    static vertex** wv;
    // wraps a bounding box around the points and generates
    // a tree.
    static dimtree_node* dimtree(vertex** vv, intT n) {
      
      // calculate bounding box
#ifdef TIME_MEASURE
      auto start = std::chrono::system_clock::now();
#endif    
      
      // copying to an array of points to make reduce more efficient
      // Do really need this? Additional copying? FUUUUUUU!
/*      parray<point> pt(n, [&] (intT i) {
        return vv[i]->pt;
      });

      assert(n >= 1);  // later: relax this constraint?
      point id = pt[0];
      point minPt = reduce(pt.begin(), pt.end(), id, [&] (point a, point b) {
        return a.min_coord(b);
      });
      point maxPt = reduce(pt.begin(), pt.end(), id, [&] (point a, point b) {
        return a.max_coord(b);
      });*/

      std::pair<point, point> id = make_pair(vv[0]->pt, vv[0]->pt);
      std::pair<point, point> borders = pasl::pctl::level1::reduce(vv, vv + n, id, [&] (std::pair<point, point> x, std::pair<point, point> y) {
        return make_pair(x.first.min_coord(y.first), x.second.max_coord(y.second));
      }, [&] (vertex* v) { return make_pair(v->pt, v->pt); });
      point minPt = borders.first;
      point maxPt = borders.second;
      //cout << "min "; minPt.print(); cout << endl;
      //cout << "max "; maxPt.print(); cout << endl;
      fvect box = maxPt - minPt;
      point center = minPt + (box / 2.0);
      
      // copy before calling recursive routine since recursive routine is destructive
//      parray<vertex*> v(vv, vv + n);
//      wv = parray<vertex*>(vv, vv + n);
      wv = (vertex**)malloc(n * sizeof(vertex*));
      pasl::pctl::pmem::copy(vv, vv + n, wv);
      //cout << "about to build tree" << endl;
#ifdef TIME_MEASURE
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<float> diff = end - start;
      printf("exectime dimtree build preparation %.3lf\n", diff.count());
#endif
      dimtree_node* result = new dimtree_node(wv, n, center, box.max_dim(), NULL, 0);

      return result;
    }
    
    int is_leaf() {
      return (vertices != NULL);
    }
    
    void del() {
      free(wv);
      wv = NULL;
      del_recursive();
    }

    void del_recursive() {
      if (is_leaf()) {
//        delete [] vertices;
      } else {
        for (int i = 0 ; i < (1 << center.dimension()); i++) {
          children[i]->del();
//          delete children[i];
        }
      }
      if (node_memory != NULL) {
        free(node_memory);
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
        parray<intT> pss;
        pss.prefix_tabulate(nb, 0);
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
    static dimtree_node* new_tree(vertex** v, intT n, point cnt, double sz, dimtree_node* new_nodes, int num_nodes) {
      return new (new_nodes) dimtree_node(v, n, cnt, sz, new_nodes + 1, num_nodes - 1);
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
      node_memory = NULL;
    }

    void build_recursive_tree(vertex** v, int n, int* offsets, int quadrants, dimtree_node* new_nodes, dimtree_node* parent, int nodes_to_left, int height, int depth) {
      parent->count = 0;

      auto comp_fct = [&] (int l, int r) {
        int lq = ((nodes_to_left << center.dimension()) + l) << (center.dimension() * (height - 1));
        int rq = ((nodes_to_left << center.dimension()) + r) << (center.dimension() * (height - 1));
        return (rq == quadrants ? n : offsets[rq]) - offsets[lq];
      };
      
      if (height == 1) {
        range::parallel_for(0, (int)(1 << center.dimension()), comp_fct, [&] (int i) {
           point new_center = (parent->center).offset_point(i, parent->size / 4.0);
           int q = (nodes_to_left << center.dimension()) + i;
           int l = ((q == quadrants - 1) ? n : offsets[q + 1]) - offsets[q];
           parent->children[i] = new_tree(v + offsets[q], l, new_center, parent->size / 2.0, new_nodes + q, 1);
        });
      } else {

//        range::parallel_for(0, (int)(1 << center.dimension()), comp_fct, [&] (int i) {
        for (int i = 0; i < 1 << center.dimension(); i++) {
//          std::cerr << "Comp_fct " << comp_fct(i, i + 1) << std::endl;
          point new_center = (parent->center).offset_point(i, parent->size / 4.0);
          parent->children[i] = new (new_nodes + i + nodes_to_left * (1 << center.dimension())) dimtree_node(new_center, parent->size / 2.0);
        }
        range::parallel_for(0, (int)(1 << center.dimension()), comp_fct, [&] (int i) {
          build_recursive_tree(v, n, offsets, quadrants, new_nodes + (1 << (depth * center.dimension())), parent->children[i], (nodes_to_left << center.dimension()) + i, height - 1, depth + 1);
        });
      }
      for (int i = 0; i < 1 << center.dimension(); i++) {
        parent->data += (parent->children[i])->data;
        parent->count += (parent->children[i])->count;
      }
      if (parent->count == 0) {
        parent->vertices = v;
        //TODO: to think about
//        parent->crazy_leaf = true;
 //       parent->count = n;
      }
    }
    
    void sort_blocks_big(vertex** v, int cnt, int quadrants, int logdivs, double size, point center, int* offsets) {
      double blocksize = size / (double)(1 << logdivs);
      point min_pt = center.offset_point(0, size / 2);
      parray<std::pair<int, vertex*>> blk(cnt, [&] (int i) {
//        std::cerr << i << " " << find_block(min_pt, blocksize, logdivs, v[i]->pt) << std::endl;
        return make_pair(find_block(min_pt, blocksize, logdivs, v[i]->pt), v[i]);
      });
      
      intsort::integer_sort(blk.begin(), offsets, cnt, quadrants, [&] (std::pair<int, vertex*>& x) { return x.first; });
      
      parallel_for(0, (int)cnt, [&] (int i) {
        v[i] = blk[i].second;
      });

    }

    void sort_blocks_small(vertex** v, int cnt, point center, int* offsets) {
      vertex* start = v[0];
      int quadrants = 1 << center.dimension();
      parray<std::pair<int, vertex*>> blk(cnt, [&] {
        for (int i = 0; i < cnt; i++) {
          blk[i] = make_pair((v[i]->pt).quadrant(center), v[i]);
        }
      });
      std::sort(blk, blk + cnt);
      int j = -1;
      for (int i = 0; i < cnt; i++) {
        v[i] = blk[i].second;
        while (blk[i].first != j) {
          offsets[++j] = i;
        }
      }
      while (++j < quadrants) offsets[j] = cnt;
      
    }

    // the recursive routine for generating the tree -- actually mutually recursive
    // due to newTree
    dimtree_node(vertex** v, intT n, point cnt, double sz, dimtree_node* new_nodes, int num_nodes) : data(node_data(cnt)) {
      //cout << "n=" << n << endl;
      count = n;
      size = sz;
      center = cnt;
      vertices = NULL;
//      crazy_leaf = false;
      node_memory = NULL;
      
      int logdivs = (int)(log2(count) * DIMTREE_SUBPROB_POW / (double)center.dimension());
      auto seq = [&] {
        int quadrants = (1 << center.dimension());
        if (count > max_leaf_size) {
          if (num_nodes < 1 << center.dimension()) {
            num_nodes = std::max(DIMTREE_ALLOC_FACTOR * std::max(count / max_leaf_size, 1 << center.dimension()), 1 << center.dimension());
            node_memory = (dimtree_node*)malloc(sizeof(dimtree_node) * num_nodes);
            new_nodes = node_memory;
          }
          intT offsets[8];
          dimtree_node* now = this;
          intsort::integer_sort(v, offsets, n, (intT)quadrants, [&] (vertex* vertex) {
            return now->find_quadrant(vertex);
          });
//          sort_blocks_small(v, n, cnt, offsets);
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
          int used_nodes = 0;
//          range::parallel_for(int(0), quadrants, comp_fct, [&] (int i) {
          for (int i = 0; i < quadrants; i++) {
            point newcenter = center.offset_point(i, size / 4.0);
            intT l = ((i == quadrants - 1) ? n : offsets[i + 1]) - offsets[i];
            int ncnt = (num_nodes - (1 << center.dimension())) * l / count + 1;
            children[i] = new_tree(v + offsets[i], l, newcenter, size / 2.0, new_nodes + used_nodes, num_nodes - ncnt);
            used_nodes += ncnt;
          }
//          });
          
/*          data = node_data(center);
          for (int i = 0; i < quadrants; i++) {
            if (children[i]->count > 0) {
              data += children[i]->data;
            }
          }*/
          for (int i = 0; i < quadrants; i++) {
            data += children[i]->data;
          }
        } else {
/*          vertices = new vertex*[count];
          data = node_data(center);
          for (intT i = 0; i < count; i++) {
            vertex* p = v[i];
            data += p;
            vertices[i] = p;
          }*/
          vertices = v;
          for (int i = 0; i < count; i++) {
            data += v[i];
          }
        }
      };
//#ifdef MANUAL_CONTROL
      if (logdivs <= 1 || count <= DIMTREE_BASE_CASE) {
        seq();
        return;
      }
//#endif
//      if (count < max_leaf_size) {
//        seq();
//        return;
//      }
//      using controller_type = pasl::pctl::granularity::controller_holder<octtree_file, 1, vertex>;
//      pasl::pctl::granularity::cstmt(controller_type::controller, [&] { return n; }, [&] {
        int divisions = (1 << logdivs); // number of quadrants in each dimension
        int quadrants = (1 << (center.dimension() * logdivs)); // total number
        int* offsets = (int*)malloc(sizeof(int) * quadrants);
        sort_blocks_big(v, count, quadrants, logdivs, this->size, center, offsets);

        num_nodes = 1 << center.dimension();
        for (int i = 0; i < logdivs; i++) {
          num_nodes = (num_nodes << center.dimension()) + (1 << center.dimension());
        }
        node_memory = (dimtree_node*)malloc(sizeof(dimtree_node) * num_nodes);

        build_recursive_tree(v, n, offsets, quadrants, node_memory, this, 0, logdivs, 1);
        free(offsets);
//      }, seq);
    }
  };

template <class intT, class pointT, class vectT, class vertexT, class dataT>
vertexT** dimtree_node<intT, pointT, vectT, vertexT, dataT>::wv = NULL;
/*parray<vertexT*> dimtree_node<intT, pointT, vectT, vertexT, dataT>::wv;*/
} // end namespace
} // end namespace

#endif // _PCTL_BENCH_OCTTREE_INCLUDED
