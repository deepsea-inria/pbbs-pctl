#include "datapar.hpp"
#include "graph.hpp"
#include "speculativefor.hpp"
#include "union.hpp"

#ifndef SPANNING_H_
#define SPANNING_H_

namespace pasl {
namespace pctl {

template <class intT>
struct unionFindStep {
  intT u;  intT v;  
  graph::edge<intT>* E;  reservation* R;  unionFind UF;

  unionFindStep() : E(NULL), R(NULL), UF(0) {};

  unionFindStep(graph::edge<intT>* _E, unionFind _UF, reservation* _R)
    : E(_E), R(_R), UF(_UF) {} 

  bool reserve(intT i) {
    u = UF.find(E[i].u);
    v = UF.find(E[i].v);
    if (u > v) {intT tmp = u; u = v; v = tmp;}
    if (u != v) {
      R[v].reserve(i);
      return 1;
    } else return 0;
  }

  bool commit(intT i) {
    if (R[v].check(i)) { UF.link(v, u); return 1; }
    else return 0;
  }
};

parray<int> spanningTree(graph::edgeArray<int> G){
  intT m = G.nonZeros;
  intT n = G.numRows;
  unionFind UF(n);
  parray<reservation> R(n);
  intT l = (4 * n) / 3;
  unionFindStep<int> UFStep(G.E, UF, R.begin()); 
  speculative_for(UFStep, 0, m, 200);
  parray<int> stIdx = filter((int*) R.begin(), (int*) R.end(), [&] (int i) { return i < INT_T_MAX; });
  std::cout << "Tree size = " << stIdx.size() << std::endl;
  UF.del();
  return stIdx;
}
} // end namespace
} // end namespace

#endif /*! SPANNING_H_ */