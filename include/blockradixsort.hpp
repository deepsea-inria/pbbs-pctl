

#include "transpose.hpp"
#include "utils.hpp"
#include "dpsdatapar.hpp"
#include "io.hpp"

#ifndef _PCTL_PBBS_BLOCKRADIXSORT_H_
#define _PCTL_PBBS_BLOCKRADIXSORT_H_

namespace pasl {
namespace pctl {
  
using namespace std;

namespace intsort {
  // Cannot be greater than 8 without changing definition of bIndexT
  //    from unsigned char to unsigned int (or unsigned short)
#define MAX_RADIX 8
#define BUCKETS 256    // 1 << MAX_RADIX

//typedef intT bucketsT[BUCKETS];


// a type that must hold MAX_RADIX bits
typedef unsigned char bIndexT;

template <class E, class F, class intT>
void radix_block(E* a, E* b, bIndexT* tmp, intT counts[BUCKETS], intT offsets[BUCKETS],
                intT offset_b, intT n, intT max_value, F extract) {
  for (intT i = 0; i < max_value; i++) {
    counts[i] = 0;
  }
  for (intT j = 0; j < n; j++) {
    intT k = tmp[j] = extract(a[j]);
    counts[k]++;
  }
  intT s = offset_b;
  for (intT i = 0; i < max_value; i++) {
    s += counts[i];
    offsets[i] = s;
  }
  for (intT j = n - 1; j >= 0; --j) {
    intT x = --offsets[tmp[j]];
    b[x] = a[j];
  }
}

template <class E, class F, class intT>
void radix_step_serial(E* a, E* b, bIndexT* tmp, intT buckets[BUCKETS],
                     intT n, intT max_value, F extract) {
  radix_block(a, b, tmp, buckets, buckets, (intT)0, n, max_value, extract);
  for (intT i = 0; i < n; i++) {
    a[i] = b[i];
  }
}

// A is the input and sorted output (length = n)
// B is temporary space for copying data (length = n)
// Tmp is temporary space for extracting the bytes (length = n)
// raw_buckets is an array of bucket sets, each set has BUCKETS integers
//    it is used for temporary space for bucket counts and offsets
// raw_buckets_number is the length of BK (number of sets of buckets)
// the first entry of BK is also used to return the offset of each bucket
// max_value is the number of buckets per set (m <= BUCKETS)
// extract is a function that extract the appropriate bits from A
//  it must return a non-negative integer less than m
template <class E, class F, class intT>
void radix_step(E* a, E* b, bIndexT *tmp, intT (*raw_buckets)[BUCKETS],
               intT raw_buckets_number, intT n, intT max_value, bool top, F extract) {
  // need 3 bucket sets per block
  intT expand = (intT) (sizeof(E) <= 4 ? 64 : 32);
  intT blocks_number = std::min(raw_buckets_number / 3, (1 + n / (BUCKETS * expand)));//(intT)std::sqrt(n) + 1);
  
  if (blocks_number < 2) {
    radix_step_serial(a, b, tmp, raw_buckets[0], n, max_value, extract);
    return;
  }
  intT block_length = (n + blocks_number - 1) / blocks_number;
  intT* cnts = (intT*) raw_buckets;
  intT* offsets_a = (intT*) (raw_buckets + blocks_number);
  intT* offsets_b = (intT*) (raw_buckets + 2 * blocks_number);
  
  parallel_for(intT(0), blocks_number, [&] (intT i) {
    intT offset = i * block_length;
    intT current_block_length = i == blocks_number - 1 ? n - offset : block_length;
    radix_block(a + offset, b, tmp + offset, cnts + max_value * i, offsets_b + max_value * i, offset, current_block_length, max_value, extract);
  });
  
  //transpose<intT,intT>(cnts, oA).trans(blocks, m);
  transpose(cnts, offsets_a, blocks_number, max_value);

//      intT ss;
  intT id = 0;
  dps::scan(offsets_a, offsets_a + blocks_number * max_value, id, [&] (intT x, intT y) {
    return x + y;
  }, offsets_a, forward_exclusive_scan);
  /*
  if (top)
    ss = sequence::scan(oA, oA, blocks*m, utils::addF<intT>(),(intT)0);
  else
    ss = sequence::scanSerial(oA, oA, blocks*m, utils::addF<intT>(),(intT)0);
   */
  //utils::myAssert(ss == n, "radixStep: sizes don't match");
  
  //blockTrans<E,intT>(B, A, oB, oA, cnts).trans(blocks, m);
  block_transpose(b, a, offsets_b, offsets_a, cnts, blocks_number, max_value);
  
  // put the offsets for each bucket in the first bucket set of raw_buckets
  for (intT j = 0; j < max_value; j++) {
    raw_buckets[0][j] = offsets_a[j * blocks_number];
  }
}

// a function to extract "bits" bits starting at bit location "offset"
template <class intT, class E, class F>
struct eBits {
  F _f;  intT _mask;  intT _offset;

  eBits(int bits, intT offset, F f): _mask((1 << bits) - 1), _offset(offset), _f(f) {}

  intT operator() (E p) {
    return _mask & (_f(p) >> _offset);
  }
};

// Radix sort with low order bits first
template <class E, class F, class intT>
void radix_loop_bottom_up(E* a, E* b, bIndexT* tmp, intT (*raw_buckets)[BUCKETS],
                       intT raw_buckets_number, intT n, int bits, bool top, F f) {
  int rounds = (bits + MAX_RADIX - 1) / MAX_RADIX;
  int bits_per_round = (bits + rounds - 1) / rounds;
  int bit_offset = 0;
  while (bit_offset < bits) {
    if (bit_offset + bits_per_round > bits) {
      bits_per_round = bits - bit_offset;
    }
    radix_step(a, b, tmp, raw_buckets, raw_buckets_number, n, (intT)1 << bits_per_round, top,
              eBits<intT, E, F>(bits_per_round, bit_offset, f));
    bit_offset += bits_per_round;
  }
}

// Radix sort with high order bits first
template <class E, class F, class intT>
void radix_loop_top_down(E* a, E* b, bIndexT* tmp, intT (*raw_buckets)[BUCKETS],
                      intT raw_buckets_number, intT n, int bits, F f) {
  if (n == 0) return;
  if (bits <= MAX_RADIX) {
    radix_step(a, b, tmp, raw_buckets, raw_buckets_number, n, (intT)1 << bits, true, eBits<intT, E, F>(bits, 0, f));
  } else if (raw_buckets_number >= BUCKETS + 1) {
    radix_step(a, b, tmp, raw_buckets, raw_buckets_number, n, (intT)BUCKETS, true,
              eBits<intT, E, F>(MAX_RADIX, bits - MAX_RADIX, f));
    intT* offsets = raw_buckets[0];
    intT remain = raw_buckets_number - BUCKETS - 1;
    float y = remain / (float) n;
    auto comp = [&] (intT l, intT r) {
      return (r == BUCKETS ? n : offsets[r]) - offsets[l];
    };
    range::parallel_for(intT(0), intT(BUCKETS), comp, [&] (intT i) {
      intT seg_offset = offsets[i];
      intT seg_next_offset = (i == BUCKETS - 1) ? n : offsets[i + 1];
      intT seg_len = seg_next_offset - seg_offset;
      intT blocks_offset = ((intT) floor(seg_offset * y)) + i + 1;
      intT blocks_next_offset = ((intT) floor(seg_next_offset * y)) + i + 2;
      intT block_len = blocks_next_offset - blocks_offset;
      radix_loop_top_down(a + seg_offset, b + seg_offset, tmp + seg_offset,
                       raw_buckets + blocks_offset, block_len, seg_len,
                       bits - MAX_RADIX, f);
    });
  } else {
    radix_loop_bottom_up(a, b, tmp, raw_buckets, raw_buckets_number, n, bits, false, f);
  }
}

template <class E, class intT>
long integer_sort_space(intT n) {
  typedef intT bucketsT[BUCKETS];
  intT blocks_number = 1 + n / (BUCKETS * 8);
  return sizeof(E) * n + sizeof(bIndexT) * n + sizeof(bucketsT) * blocks_number;
}

// Sorts the array A, which is of length n.
// Function f maps each element into an integer in the range [0,max_value)
// If bucketOffsets is not NULL then it should be an array of length max_value
// The offset in A of each bucket i in [0, max_value) is placed in location i
//   such that for i < max_value - 1, offsets[i + 1] - offsets[i] gives the number
//   of keys=i.   For i = max_value - 1, n-offsets[i] is the number.
template <class E, class F, class intT>
void integer_sort(E *a, intT* bucket_offsets, intT n, intT max_value, bool bottom_up,
           char* tmp_space, F f) {
  
  typedef intT bucketsT[BUCKETS];
  
  int bits = pasl::pctl::utils::log2Up(max_value);
  intT raw_buckets_number = 1 + n / (BUCKETS * 8);
  
  // the temporary space is broken into 3 parts: B, Tmp and raw_buckets
  E* b = (E*) tmp_space;
  intT b_size = sizeof(E) * n;
  bIndexT* tmp = (bIndexT*) (tmp_space + b_size); // one byte per item
  intT tmp_size = sizeof(bIndexT) * n;
  bucketsT* raw_buckets = (bucketsT*) (tmp_space + b_size + tmp_size);
  if (bits <= MAX_RADIX) {
    radix_step(a, b, tmp, raw_buckets, raw_buckets_number, n, (intT) 1 << bits, true, eBits<intT, E, F>(bits, 0, f));
    if (bucket_offsets != NULL) {
      pmem::copy(raw_buckets[0], raw_buckets[0] + max_value, bucket_offsets);
      
/*      parallel_for(intT(0), max_value, [&] (intT i) {
        bucket_offsets[i] = raw_buckets[0][i];
      }); */
    }
    return;
  } else if (bottom_up) {
    radix_loop_bottom_up(a, b, tmp, raw_buckets, raw_buckets_number, n, bits, true, f);
  } else {
    radix_loop_top_down(a, b, tmp, raw_buckets, raw_buckets_number, n, bits, f);
  }
  // Filling offsets array
  if (bucket_offsets != NULL) {
    pmem::fill(bucket_offsets, bucket_offsets + max_value, n);
//    { parallel_for(intT(0), m, [&] (intT i) { bucket_offsets[i] = n; }); }
    {
      parallel_for(intT(0), n - 1, [&] (intT i) {
        intT v = f(a[i]);
        intT vn = f(a[i + 1]);
        if (v != vn) {
          bucket_offsets[vn] = i + 1;
        }
      });
    }
    bucket_offsets[f(a[0])] = 0;
    dps::scan(bucket_offsets, bucket_offsets + max_value, n, [&] (intT x, intT y) {
      return std::min(x, y);
    }, bucket_offsets, backward_inclusive_scan);
/*        sequence::scanIBack(bucket_offsets, bucket_offsets, (intT) m,
                        utils::minF<intT>(), (intT) n); */
  }
}

template <class E, class F, class intT>
void integer_sort(E* a, intT* bucket_offsets, intT n, intT max_value, bool bottom_up, F f) {
  long x = integer_sort_space<E, intT>(n);
#ifdef MANUAL_ALLOCATION
  char* s = (char*) malloc(x);
  integer_sort(a, bucket_offsets, n, max_value, bottom_up, s, f);
  free(s);
#else
  parray<char> s;
  s.prefix_tabulate(x, 0);
  integer_sort(a, bucket_offsets, n, max_value, bottom_up, s.begin(), f);
#endif
}

template <class E, class F, class intT>
void integer_sort(E* a, intT* bucket_offsets, intT n, intT max_value, F f) {
  integer_sort(a, bucket_offsets, n, max_value, false, f);
}

// A version that uses a NULL bucketOffset
template <class E, class Func, class intT>
void integer_sort(E* a, intT n, intT max_value, Func f) {
  integer_sort(a, (intT*) NULL, n, max_value, false, f);
}

template <class E, class F, class intT>
void integer_sort_bottom_up(E* a, intT n, intT max_value, F f) {
  integer_sort(a, (intT*) NULL, n, max_value, true, f);
}
  
 
} // end namespace
  
template <class intT, class uintT>
static void integer_sort(uintT* a, intT n) {
  intT max_value = pasl::pctl::max(a, a + n);
  intsort::integer_sort(a, (intT*) nullptr, n, max_value + 1, [&] (uintT x) { return x; });
}

template <class T, class intT, class uintT>
static void integer_sort(std::pair<uintT, T>* a, intT n) {
  intT max_value = pasl::pctl::level1::reduce(a, a + n, 0, [&] (uintT a, uintT b) {
    return std::max(a, b);
  }, [&] (std::pair<uintT, T>& x) { return 1; }, [&] (std::pair<uintT, T>& x) { return x.first; });
  intsort::integer_sort(a, (intT*) nullptr, n, max_value + 1, [&] (std::pair<uintT, T> x) { return x.first; });
}

  
} // end namespace
} // end namespace


#endif /*! _PCTL_PBBS_BLOCKRADIXSORT_H_ */