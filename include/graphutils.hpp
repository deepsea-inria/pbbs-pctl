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
#include <fstream>
#include <cstdlib>
#include <math.h>

#include "graph.hpp"
#include "prandgen.hpp"

#ifndef _GRAPH_UTILS_INCLUDED
#define _GRAPH_UTILS_INCLUDED

namespace pasl {
namespace pctl {
namespace graph {

using namespace std;

template <class intT>
edgeArray<intT> to_edge_array(graph<intT>& G) {
  int num_rows = G.n;
  int non_zeros = G.m;
  vertex<intT>* v = G.V;
  edge<intT>* e = newA(edge<intT>, non_zeros);

  int k = 0;
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < v[i].degree; j++) {
      if (i < v[i].Neighbors[j]) {
        e[k++] = edge<int>(i, v[i].Neighbors[j]);
      }
    }
  }
  return edgeArray<intT>(e, num_rows, num_rows, non_zeros);
}

template <class intT>
wghEdgeArray<intT> to_weighted_edge_array(graph<intT>& G) {
  int n = G.n;
  int m = G.m;
  vertex<intT>* v = G.V;
  wghEdge<intT>* e = newA(wghEdge<intT>, m);

  int k = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < v[i].degree; j++) {
      if (i < v[i].Neighbors[j]) {
        e[k++] = wghEdge<intT>(i, v[i].Neighbors[j], prandgen::hashi(k));
      }
    }
  }
  return wghEdgeArray<int>(e, n, m);
}

} // end namespace
} // end namespace
} // end namespace

#endif // _GRAPH_UTILS_INCLUDED
