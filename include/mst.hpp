#include "datapar.hpp"
#include "graph.hpp"
#include "speculativefor.hpp"
#include "union.hpp"
#include "samplesort.hpp"

#ifndef MST_H_
#define MST_H_

namespace pasl {
namespace pctl {

struct indexedEdge {
  int u;
  int v;
  int id;

  indexedEdge() : u(0), v(0), id(0) {}
  indexedEdge(int _u, int _v, int _id) : u(_u), v(_v), id(_id) {}
};

struct UnionFindStep {
  int u;  int v;  
  indexedEdge* E;  reservation* R;  unionFind UF;  bool* inST;

  UnionFindStep() : E(NULL), R(NULL), UF(0), inST(NULL) {};
  UnionFindStep(indexedEdge* _E, unionFind _UF, reservation* _R, bool* ist) 
    : E(_E), R(_R), UF(_UF), inST(ist) {}

  bool reserve(int i) {
    u = UF.find(E[i].u);
    v = UF.find(E[i].v);
    if (u != v) {
      R[v].reserve(i);
      R[u].reserve(i);
      return 1;
    } else return 0;
  }

  bool commit(int i) {
    if (R[v].check(i)) {
      R[u].checkReset(i); 
      UF.link(v, u); 
      inST[E[i].id] = 1;
      return 1;
    } else if (R[u].check(i)) {
      UF.link(u, v); 
      inST[E[i].id] = 1;
      return 1;
    } else return 0;
  }
};

template <class E, class F>
int almostKth(E* A, E* B, int k, int n, F f) {
  int ssize = std::min(1000, n);
  int stride = n / ssize;
  int km = (int) (k * ((double) ssize) / n);
  E T[ssize];
  for (int i = 0; i < ssize; i++) T[i] = A[i * stride];
  std::sort(T, T + ssize, f);
  E p = T[km];

  parray<bool> flags(n, [&] (int i) {
    return flags[i] = f(A[i], p);
  });
  int l = dps::pack(flags.begin(), A, A + n, B);
  parallel_for(0, n, [&] (int i) {
    flags[i] = !flags[i];
  });
  dps::pack(flags.begin(), A, A + n, B + l);
  return l;
}

typedef std::pair<double, int> ei;

struct edgeLess {
  bool operator() (ei a, ei b) { 
    return (a.first == b.first) ? (a.second < b.second) 
      : (a.first < b.first);}};

parray<long> mst(graph::wghEdgeArray<int> G) { 
  graph::wghEdge<int>* E = G.E;
  parray<ei> x(G.m, [&] (int i) {
    return ei(E[i].weight, i);
  });

  int l = std::min(4 * G.n / 3, G.m);
  parray<ei> y;
  y.prefix_tabulate(G.m, 0);

  l = almostKth(x.begin(), y.begin(), l, G.m, edgeLess());

  sample_sort(y.begin(), l, edgeLess());

  unionFind UF(G.n);
  parray<reservation> R(G.n);
  //nextTime("initialize nodes");

  parray<indexedEdge> z;
  z.prefix_tabulate(G.m, 0);
  parallel_for(0, l, [&] (int i) {
    int j = y[i].second;
    z[i] = indexedEdge(E[j].u, E[j].v, j);
  });
  //nextTime("copy to edges");

  parray<bool> mstFlags(G.m, (bool) 0);

  UnionFindStep UFStep(z.begin(), UF, R.begin(), mstFlags.begin());
  speculative_for(UFStep, 0, l, 100);

  parray<bool> flags(G.m - l, [&] (int i) {
    int j = y[i + l].second;
    int u = UF.find(E[j].u);
    int v = UF.find(E[j].v);
    if (u != v) return 1;
    else return 0;
  });

  intT k = dps::pack(flags.begin(), y.begin() + l, y.begin() + G.m, x.begin());
  flags.clear();
  y.clear();

  sample_sort(x.begin(), k, edgeLess());

  z.prefix_tabulate(k, 0);
  parallel_for(0, k, [&] (int i) {
    int j = x[i].second;
    z[i] = indexedEdge(E[j].u, E[j].v, j);
  });
  x.clear();

  UFStep = UnionFindStep(z.begin(), UF, R.begin(), mstFlags.begin());
  speculative_for(UFStep, 0, k, 20);

  z.clear(); 

  parray<long> mst = pack_index(mstFlags.begin(), mstFlags.end());
  mstFlags.clear();

  std::cout << "n=" << G.n << " m=" << G.m << " nInMst=" << mst.size() << std::endl;
  UF.del();
  return mst;
}

} // end namespace
} // end namespace

#endif /*! MST_H_ */