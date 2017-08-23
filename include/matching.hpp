#include <iostream>
#include "datapar.hpp"
#include "graph.hpp"
#include "utils.hpp"
#include "speculativefor.hpp"

#ifndef MATCHING_H_
#define MATCHING_H_

namespace pasl {
namespace pctl {

struct matchStep {
  graph::edge<int>* E;  
  int* R;  
  bool* matched;

  matchStep() : E(NULL), R(NULL), matched(NULL) {}

  matchStep(graph::edge<int>* _E, int* _R, bool* m) : E(_E), R(_R), matched(m) {}

  bool reserve(int i) {
    int u = E[i].u;
    int v = E[i].v;
    if (matched[u] || matched[v] || (u == v)) return 0;
    reserveLoc(R[u], i);
    reserveLoc(R[v], i);
    return 1;
  }

  bool commit(int i) {
    int u = E[i].u;
    int v = E[i].v;
    if (R[v] == i) {
      R[v] = INT_T_MAX;
      if (R[u] == i) {
	matched[u] = matched[v] = 1;
	return 1;
      } 
    } else if (R[u] == i) R[u] = INT_T_MAX;
    return 0;
  }
};

parray<int> maximalMatching(graph::edgeArray<int> G) {
  int n = std::max(G.numCols, G.numRows);
  int m = G.nonZeros;
  parray<int> r(n, (int) INT_T_MAX);
  parray<bool> matched(n, (bool) 0);
  matchStep mStep(G.E, r.begin(), matched.begin());
  speculative_for(mStep, 0, m, 150, 0);
  parray<int> matchingIdx = filter(r.begin(), r.end(), [&] (int i) { return i < INT_T_MAX; });
  std::cout << "number of matches = " << matchingIdx.size() << std::endl;
  return matchingIdx;
}  

} // end namespace
} // end namespace
#endif /*! MATCHING_H_ */