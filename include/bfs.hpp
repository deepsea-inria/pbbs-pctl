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

#include "utils.hpp"
#include "datapar.hpp"
#include "graph.hpp"
#include "timer.hpp"
#include "sequence.h"
#undef parallel_for
namespace pasl {
namespace pctl {

using namespace std;

// **************************************************************
//    Non-DETERMINISTIC BREADTH FIRST SEARCH
// **************************************************************

// **************************************************************
//    THE NON-DETERMINISTIC BSF
//    Updates the graph so that it is the BFS tree (i.e. the neighbors
//      in the new graph are the children in the bfs tree)
// **************************************************************

struct nonNegF{bool operator() (int a) {return (a>=0);}};

pair<int,int> bfs(int start, graph::graph<int> graph) {
  timer init_timer;
  init_timer.start();
  int numVertices = graph.n;
  int numEdges = graph.m;
  graph::vertex<int>* g = graph.V;
#ifdef MANUAL_ALLOCATION
  int* frontier = (int*)malloc(sizeof(int) * numEdges);
  int* visited = (int*)malloc(sizeof(int) * numVertices);
  pmem::fill(visited, visited + numVertices, 0);
  int* frontier_next = (int*)malloc(sizeof(int) * numEdges);

  int* counts = (int*)malloc(sizeof(int) * numVertices);
#else
  parray<int> frontier;
  frontier.prefix_tabulate(numEdges, 0);
  parray<int> visited(numVertices, 0);
  parray<int> frontier_next;
  frontier_next.prefix_tabulate(numEdges, 0);
  parray<int> counts;
  counts.prefix_tabulate(numVertices, 0);
#endif
  
  frontier[0] = start;
  int frontier_size = 1;
  visited[start] = 1;

  int total_visited = 0;
  int round = 0;
  init_timer.end("initialization");
#ifdef MANUAL_ALLOCATION
  auto visited_ptr = visited;
  auto frontier_ptr = frontier;
  auto frontier_next_ptr = frontier_next;
  auto counts_ptr = counts;
#else
  auto visited_ptr = visited.begin();
  auto frontier_ptr = frontier.begin();
  auto frontier_next_ptr = frontier_next.begin();
  auto counts_ptr = counts.begin();
#endif

  timer scan_timer;
  timer main_timer;
  timer filter_timer;
  while (frontier_size > 0) {
    round++;
    total_visited += frontier_size;
//    std::cerr << total_visited << std::endl;
//    std::cerr << frontier_size << std::endl;
    scan_timer.start();
//    parallel_for(0, frontier_size, [&] (int i) {
//      cilk_for (int i = 0; i < frontier_size; i++)
//      for (int i = 0; i < frontier_size; i++)
//	counts[i] = g[frontier[i]].degree;
//    });

    range::parallel_for(0, frontier_size, [&] (int l, int r) { return r - l; }, [&, counts_ptr, g, frontier_ptr] (int i) {
        counts_ptr[i] = g[frontier_ptr[i]].degree;
      }, [&, counts_ptr, g, frontier_ptr] (int l, int r) {
        for (int i = l; i < r; i++) {
          counts_ptr[i] = g[frontier[i]].degree;
        }
      }
    );
#ifdef MANUAL_ALLOCATION
#ifdef PBBS_SEQUENCE
    int nr = pbbs::sequence::scan(counts, counts, frontier_size, [&] (int x, int y) { return x + y; }, 0);
#else
    int nr = dps::scan(counts, counts + frontier_size, 0, [&] (int x, int y) { return x + y; }, counts, forward_exclusive_scan);
#endif
#else
#ifdef PBBS_SEQUENCE
    int nr = pbbs::sequence::scan(counts.begin(), counts.begin(), frontier_size, [&] (int x, int y) { return x + y; }, 0);
#else
    int nr = dps::scan(counts.begin(), counts.begin() + frontier_size, 0, [&] (int x, int y) { return x + y; }, counts.begin(), forward_exclusive_scan);
#endif
#endif
    scan_timer.end();

    main_timer.start();
    // For each vertexB in the frontier try to "hook" unvisited neighbors.
//    range::parallel_for(0, frontier_size, [&] (int l, int r) { return (r == frontier_size ? nr : counts[r]) - counts[l]; }, [&] (int i) {
//    cilk_for (int i = 0; i < frontier_size; i++) {
//    for (int i = 0; i < frontier_size; i++) {
     range::parallel_for(0, frontier_size, [&] (int l, int r) { return (r == frontier_size ? nr : counts_ptr[r]) - counts_ptr[l] + (r - l); }, [&, frontier_next_ptr, frontier_ptr, g, visited_ptr] (int i) {
       int k = 0;
       int v = frontier_ptr[i];
       int o = counts_ptr[i];

       for (int j = 0; j < g[v].degree; j++) {
         int ngh = g[v].Neighbors[j];
           if (visited_ptr[ngh] == 0 && !__sync_val_compare_and_swap(&visited_ptr[ngh], 0, 1)) {//utils::CAS(&visited_ptr[ngh], 0, 1)) {
             frontier_next_ptr[o + j] = g[v].Neighbors[k++] = ngh;
           }
         else frontier_next_ptr[o + j] = -1;}
        g[v].degree = k;
     }, [&, frontier_next_ptr, frontier_ptr, g, visited_ptr] (int l, int r) {
       for (int i = l; i < r; i++) {
         int k = 0;
         int v = frontier_ptr[i];
         int o = counts_ptr[i];

        for (int j = 0; j < g[v].degree; j++) {
          int ngh = g[v].Neighbors[j];
          if (visited_ptr[ngh] == 0 && !__sync_val_compare_and_swap(&visited_ptr[ngh], 0, 1)) {
            frontier_next_ptr[o + j] = g[v].Neighbors[k++] = ngh;
          }
          else frontier_next_ptr[o + j] = -1;
        }
        g[v].degree = k;
      }
    });
//      }}

    main_timer.end();
    filter_timer.start();
    // Filter out the empty slots (marked with -1)
#ifdef MANUAL_ALLOCATION
#ifdef PBBS_SEQUENCE
    frontier_size = pbbs::sequence::filter(frontier_next, frontier, nr, [&] (int v) { return v >= 0; });
#else
    frontier_size = dps::filter(frontier_next, frontier_next + nr, frontier, [&] (int v) { return v >= 0; });
#endif
#else
#ifdef PBBS_SEQUENCE
    frontier_size = pbbs::sequence::filter(frontier_next.begin(), frontier.begin(), nr, [&] (int v) { return v >= 0; });
#else
    frontier_size = dps::filter(frontier_next.begin(), frontier_next.begin() + nr, frontier.begin(), [&] (int v) { return v >= 0; });
#endif
#endif
    filter_timer.end();
  }
  std::cerr << total_visited << " " << round << std::endl;
  scan_timer.report_total("scan total");
  main_timer.report_total("main total");
  filter_timer.report_total("filter total");
  return pair<int, int>(total_visited, round);
}
} //end namespace
} //end namespace
