// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and Harsha Vardhan Simhadri and the PBBS team
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
#include <algorithm>
#include "utils.hpp"
#include <math.h>
#include "quicksort.hpp"
#include "transpose.hpp"
#include "prandgen.hpp"

#ifndef _PBBS_PCTL_SAMPLESORT_H_
#define _PBBS_PCTL_SAMPLESORT_H_

#ifdef MANUAL_ALLOCATION
#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))
#endif

namespace pasl {
namespace pctl {
  
template<class E, class BinPred, class intT>
void split_positions(E* a, E* b, intT* c, intT length_a, intT length_b, BinPred compare) {
  if (length_a == 0 || length_b == 0) {
    return;
  }
  int pos_a = 0;
  int pos_b = 0;
  int pos_c = 0;
  for (intT i = 0; i < 2 * length_b; i++) {
    c[i] = 0;
  }
  while (pos_b < length_b) {
    while (pos_a < length_a && compare(a[pos_a], b[pos_b])) {
      c[pos_c]++;
      pos_a++;
    }
    pos_c++;
    while (pos_a < length_a && (compare(a[pos_a], b[pos_b]) ^ compare(b[pos_b], a[pos_a]) ^ true)) {
      c[pos_c]++;
      pos_a++;
    }

    pos_b++;
    pos_c++;
    // The pivots are equal
    while (pos_b < length_b && !compare(b[pos_b - 1], b[pos_b])) {
      pos_b++;
      pos_c += 2;
    }
  }
  c[pos_c] = length_a - pos_a;
}

#define SSORT_THR 100000
#define AVG_SEG_SIZE 2
#define PIVOT_QUOT 2
#define comparison_sort(__A, __n, __compare) (quickSort(__A, __n, __compare))

template <class E, class BinPred, class intT>
class samplesort_contr {
public:
  static controller_type contr;
};

template <class E, class BinPred, class intT>
controller_type samplesort_contr<E,BinPred,intT>::contr("samplesort");

template<class E, class BinPred, class intT>
void sample_sort (E* a, intT n, BinPred compare) {
//  std::cerr << "Right here " << n << "\n";
  using controller_type = samplesort_contr<E, BinPred, intT>;
  par::cstmt(controller_type::contr, [&] { return (int) (n * log(n)); }, [&] {
#ifdef MANUAL_CONTROL
    if (n <= SSORT_THR) {
      comparison_sort(a, n, compare);
      return;
    }
#endif
    if (n <= 1) {
      return;
    }
    intT sq = (intT) sqrt(n);
    intT row_length = sq * AVG_SEG_SIZE;
    intT rows = (intT) ceil(1. * n / row_length);
    // number of pivots + 1
    intT segments = (sq - 1) / PIVOT_QUOT;
    if (segments <= 1) {
      std::sort(a, a + n, compare);
      return;
    }

    int over_sample = 4;
    intT sample_set_size = segments * over_sample;
//    std::cerr << a << " " << n << " " << sample_set_size << " " << rows << " " << segments << " " << sq << " " << row_length << std::endl;
    // generate samples with oversampling
#ifdef MANUAL_ALLOCATION
    E* sample_set = newA(E, sample_set_size);
    parallel_for(0, sample_set_size, [&] (int i) {
      intT o = prandgen::hashi(i) % n;
      sample_set[i] = a[o];
    });
#else
    parray<E> sample_set(sample_set_size, [&] (intT j) {
      intT o = prandgen::hashi(j) % n;
      return a[o];
    });
#endif
    //cout << "n=" << n << " num_segs=" << segments << endl;
      
    // sort the samples
#ifdef MANUAL_ALLOCATION
    quickSort(sample_set, sample_set_size, compare);
//    std::sort(sample_set, sample_set + sample_set_size, compare);
#else
    quickSort(sample_set.begin(), sample_set_size, compare);
//    std::sort(sample_set.begin(), sample_set.end(), compare);
#endif
      
    // subselect samples at even stride
    int pivots_size = segments - 1;
#ifdef MANUAL_ALLOCATION
    E* pivots = newA(E, pivots_size);
    parallel_for(0, segments - 1, [&] (int i) {
      intT o = over_sample * i;
      pivots[i] = sample_set[o];
    });
#else
    parray<E> pivots(segments - 1, [&] (intT k) {
      intT o = over_sample * k;
      return sample_set[o];
    });
#endif
    //nextTime("samples");
#ifdef MANUAL_ALLOCATION
    free(sample_set);
#endif
            
    segments = 2 * segments - 1;
#ifdef MANUAL_ALLOCATION
    E* b = newA(E, rows * row_length);
    intT* segments_sizes = newA(intT, rows * segments);
    intT* offset_a = newA(intT, rows * segments);
    intT* offset_b = newA(intT, rows * segments);
#else
    parray<E> b(rows * row_length);
    parray<intT> segments_sizes(rows * segments);
    parray<intT> offset_a(rows * segments);
    parray<intT> offset_b(rows * segments);
#endif
    
    // sort each row and merge with samples to get counts
    range::parallel_for((intT)0, rows, [&] (intT lo, intT hi) { return (hi - lo) * row_length; }, [&] (intT r) {
      intT offset = r * row_length;
      intT size = (r < rows - 1) ? row_length : n - offset;
      sample_sort(a + offset, size, compare);
//      std::sort(a + offset, a + offset + size, compare);
#ifdef MANUAL_ALLOCATION
      split_positions(a + offset, pivots, segments_sizes + r * segments, size, pivots_size, compare);
#else
      split_positions(a + offset, pivots.begin(), segments_sizes.begin() + r * segments, size, (intT)pivots.size(), compare);
#endif
    });
    //nextTime("sort and merge");
//    std::cerr << pivots << "\n";
//    std::cerr << segments << " " << rows << " " << segments_sizes << "\n";

    // transpose from rows to columns
    auto plus = [&] (intT x, intT y) {
      return x + y;
    };
#ifdef MANUAL_ALLOCATION
    dps::scan(segments_sizes, segments_sizes + rows * segments, (intT)0, plus, offset_a, forward_exclusive_scan);
    transpose(segments_sizes, offset_b, rows, segments);
    dps::scan(offset_b, offset_b + rows * segments, (intT)0, plus, offset_b, forward_exclusive_scan);
    block_transpose(a, b, offset_a, offset_b, segments_sizes, rows, segments);
    pmem::copy(b, b + n, a);

    free(b);
    free(offset_a);
    free(segments_sizes);
#else
    dps::scan(segments_sizes.begin(), segments_sizes.end(), (intT)0, plus, offset_a.begin(), forward_exclusive_scan);
    transpose(segments_sizes.begin(), offset_b.begin(), rows, segments);
    dps::scan(offset_b.begin(), offset_b.end(), (intT)0, plus, offset_b.begin(), forward_exclusive_scan);
    block_transpose(a, b.begin(), offset_a.begin(), offset_b.begin(), segments_sizes.begin(), rows, segments);
    pmem::copy(b.begin(), b.begin() + n, a);
    //nextTime("transpose");
#endif

    // sort the columns
    auto complexity_fct = [&] (intT lo, intT hi) {
      if (lo == hi) {
        return 0;
      } else if (hi < pivots_size) {
        return offset_b[(2 * hi + 1) * rows] - offset_b[2 * lo * rows];//(int) ((offset_b[(2 * hi + 1) * rows] - offset_b[2 * lo * rows]) * log(offset_b[(2 * hi + 1) * rows] - offset_b[2 * lo * rows]));
      } else {
        return n - offset_b[2 * lo * rows];//(int) ((n - offset_b[2 * lo * rows]) * log(n - offset_b[2 * lo * rows]));
      }
    };
    range::parallel_for((intT)0, (intT)(pivots_size + 1), complexity_fct, [&] (intT i) {
//    range::parallel_for((intT)0, (intT)(pivots.size() + 1), [&] (intT i) {
      intT offset = offset_b[(2 * i) * rows];
      if (i == 0) {
          sample_sort(a, offset_b[rows], compare); // first segment
//          std::sort(a, a + offset_b[rows], compare); // first segment
      } else if (i < pivots_size) { // middle segments
        // if not all equal in the segment
        if (compare(pivots[i - 1], pivots[i])) {
          sample_sort(a + offset, offset_b[(2 * i + 1) * rows] - offset, compare);
//            std::sort(a + offset, a + offset_b[(i + 1) * rows], compare);
        }
      } else { // last segment
          sample_sort(a + offset, n - offset, compare);
//          std::sort(a + offset, a + n, compare);
      }
    });
    //nextTime("last sort");
#ifdef MANUAL_ALLOCATION
    free(pivots);
    free(offset_b);
#endif
  }, [&] {
    std::sort(a, a + n, compare);
  });
}
  
} // end namespace
} // end namespace

#undef compSort
#define compSort(__A, __n, __f) (sampleSort(__A, __n, __f))

#endif /*! _PBBS_PCTL_SAMPLESORT_H_ !*/