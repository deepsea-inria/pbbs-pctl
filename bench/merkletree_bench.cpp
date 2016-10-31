/*!
 * \file merkletree.cpp
 * \brief Example use of pctl granularity control
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <openssl/sha.h>
#include <climits>
#include <type_traits>

#include "bench.hpp"
#include "prandgen.hpp"

/***********************************************************************/

namespace cmdline = deepsea::cmdline;

template <class T>
T* malloc_array(size_t n) {
  return (T*)malloc(sizeof(T) * n);
}

namespace pasl {
namespace pctl {

using namespace granularity;
  
template <class Digest, class Hash_fn>
void hash2(Digest& lhs, Digest& rhs, Digest& dst, const Hash_fn& hash_fn) {
  Digest tmp;
  for (int i = 0; i < Digest::szb1; i++) {
    tmp[i] = lhs[i] | rhs[i];
  }
  hash_fn((const unsigned char*)tmp.begin(), (size_t)Digest::szb1, dst.begin());
}

#if 0
template <class Digest, class Hash_fn>
void hash_range_seq(Digest* lo, Digest* hi, Digest& dst, const Hash_fn& hash_fn) {
  auto n = hi - lo;
  if (n == 0) {
    memset(dst.begin(), 0, Digest::szb1);
  } else if (n == 1) {
    pasl::pctl::pmem::copy(lo->begin(), lo->end(), dst.begin());
  } else {
    auto mid = lo + (n / 2);
    Digest dst1, dst2;
    hash_range_seq(lo, mid, dst1, hash_fn);
    hash_range_seq(mid, hi, dst2, hash_fn);
    hash2(dst1, dst2, dst, hash_fn);
  }
}

template <class Hash_fn, class Digest>
class hash_output {
public:
  
  using result_type = Digest;
  
  Hash_fn hash_fn;
  
  hash_output(const Hash_fn& hash_fn)
  : hash_fn(hash_fn) { }
  
  void init(result_type& dst) const {
    memset(dst.begin(), 0, Digest::szb1);
  }
  
  void copy(const result_type& src, result_type& dst) const {
    pasl::pctl::pmem::copy(src.begin(), src.end(), dst.begin());
  }
  
  // dst, src represent left, right range to be merged, respectively
  void merge(result_type& src, result_type& dst) const {
    hash2(dst, src, dst, hash_fn);
  }
  
};

template <class Digest, class Hash_fn>
void hash_range_par(Digest* lo, Digest* hi, Digest& dst, const Hash_fn& hash_fn) {
  using result_type = Digest;
  using output_type = hash_output<decltype(hash_fn), result_type>;
  output_type out(hash_fn);
  result_type id;
  memset(id.begin(), 0, Digest::szb1);
  memset(dst.begin(), 0, Digest::szb1);
  auto lift_comp_rng = [&] (Digest* lo, Digest* hi) {
    return (hi - lo) * Digest::szb1;
  };
  auto lift_idx_dst = [&] (long, Digest& src, result_type& dst) {
    hash2(dst, src, dst, hash_fn);
  };
  auto seq_reduce_rng_dst = [&] (Digest* lo, Digest* hi, result_type& dst) {
    hash_range_seq(lo, hi, dst, hash_fn);
  };
  pasl::pctl::level3::reduce(lo, hi, out, id, dst, lift_comp_rng, lift_idx_dst, seq_reduce_rng_dst);
}

template <class Digest, class Hash_fn>
void hash_range1(const unsigned char* lo, const unsigned char* hi, Digest& dst, const Hash_fn& hash_fn) {
  auto n = hi - lo;
  auto m = n / sizeof(Digest);
  auto lo2 = (Digest*)lo;
  auto hi2 = lo2 + m;
  hash_range_par(lo2, hi2, dst, hash_fn);
}
  
#endif
  
template <class Int>
Int left_child(Int i) {
  return 2 * i;
}

template <class Int>
Int right_child(Int i) {
  return 2 * i + 1;
}
  
template <class Int>
Int parent(Int i) {
  return i >> 1;
}

template <class Block_iterator, class Hash, class Hash_fn, class Hash_block_fn>
void merkletree_seq(Block_iterator lo, Block_iterator hi, Hash* merkle_lo,
                    const Hash_fn& hash_fn,
                    const Hash_block_fn& hash_block_fn) {
  auto row_lo = hi - lo;
  int i = 0;
  for (auto it = lo; it != hi; it++, i++) {
    hash_block_fn(it, merkle_lo + row_lo + i);
  }
  assert(i == row_lo);
  for (row_lo = parent(row_lo); row_lo != 0; row_lo = parent(row_lo)) {
    for (auto i = row_lo; i < 2 * row_lo; i++) {
      hash_fn(merkle_lo + left_child(i), merkle_lo + right_child(i), merkle_lo + i);
    }
  }
}
  
template <class Block_iterator, class Hash, class Hash_fn, class Hash_block_fn>
void merkletree_par(Block_iterator lo, Block_iterator hi, Hash* merkle_lo,
                    const Hash_fn& hash_fn,
                    const Hash_block_fn& hash_block_fn) {
  using value_type = typename std::iterator_traits<Block_iterator>::value_type;
  auto row_lo = hi - lo;
  pasl::pctl::range::parallel_for(lo, hi, [&] (Block_iterator lo, Block_iterator hi) {
    return (hi - lo) * sizeof(value_type);
  }, [&] (Block_iterator it) {
    hash_block_fn(it, merkle_lo + row_lo + (it - lo));
  }, [&] (Block_iterator lo, Block_iterator hi) {
    for (auto it = lo; it != hi; it++) {
      hash_block_fn(it, merkle_lo + row_lo + (it - lo));
    }
  });
  for (row_lo = parent(row_lo); row_lo != 0; row_lo = parent(row_lo)) {
    pasl::pctl::range::parallel_for(row_lo, 2 * row_lo, [&] (unsigned lo, unsigned hi) {
      return (hi - lo) * sizeof(Hash);
    }, [&] (unsigned i) {
      hash_fn(merkle_lo + left_child(i), merkle_lo + right_child(i), merkle_lo + i);
    }, [&] (unsigned lo, unsigned hi) {
      for (unsigned i = lo; i != hi; i++) {
        hash_fn(merkle_lo + left_child(i), merkle_lo + right_child(i), merkle_lo + i);
      }
    });
  }
}
  
template <class Digest, class Hash_fn, class Hash_block_fn>
void hash_range2(Digest* lo, Digest* hi, Digest& dst,
                 const Hash_fn& hash_fn,
                 const Hash_block_fn& hash_block_fn) {
  auto n = hi - lo;
  Digest* merkle = malloc_array<Digest>(2 * n);
  merkletree_par(lo, hi, merkle, hash_fn, hash_block_fn);
  auto root_hash = merkle + 1;
  pasl::pctl::pmem::copy(root_hash->begin(), root_hash->end(), dst.begin());
  free(merkle);
}
  
template <class Digest, class Hash_fn>
void hash_range3(const unsigned char* lo, const unsigned char* hi, Digest& dst,
                 const Hash_fn& hash_fn) {
  auto n = hi - lo;
  auto m = n / sizeof(Digest);
  auto lo2 = (Digest*)lo;
  auto hi2 = lo2 + m;
  auto hash_fn2 = [&] (const Digest* src1, const Digest* src2, Digest* dst) {
    hash2(*((Digest*)src1), *((Digest*)src2), *dst, hash_fn);
  };
  auto hash_block_fn = [&] (const Digest* lo,  Digest* dst) {
    hash_fn((const unsigned char*)lo, sizeof(Digest), (unsigned char*)dst);
  };
  hash_range2(lo2, hi2, dst, hash_fn2, hash_block_fn);
}
  
template <class Block_iterator, class Hash_policy>
typename Hash_policy::digest_type* merkletree(pbbs::measured_type measure,
                                              Block_iterator lo, Block_iterator hi,
                                              const Hash_policy& hash_policy,
                                              unsigned& nb) {
  using hash_type = typename Hash_policy::digest_type;
  auto n = hi - lo;
  nb = (unsigned)(2 * n);
  hash_type* merkle = malloc_array<hash_type>(nb);
  auto hash_fn = [&] (hash_type* lhs, hash_type* rhs, hash_type* dst) {
    Hash_policy::hash_2(lhs, rhs, dst);
  };
  auto hash_block_fn = [&] (Block_iterator block, hash_type* dst) {
    const unsigned char* lo = (const unsigned char*)(&block[0]);
    auto hi = lo + sizeof(typename std::iterator_traits<Block_iterator>::value_type);
    Hash_policy::hash_range(lo, hi, dst);
  };
  cmdline::dispatcher d;
  d.add("sequential", [&] {
    measure([&] {
      merkletree_seq(lo, hi, merkle, hash_fn, hash_block_fn);
    });
  });
  d.add("parallel", [&] {
    measure([&] {
      merkletree_par(lo, hi, merkle, hash_fn, hash_block_fn);
    });
  });
  d.dispatch("algorithm");
  if (cmdline::parse_or_default_bool("check", false)) {
    hash_type* merkle_ref = malloc_array<hash_type>(nb);
    merkletree_seq(lo, hi, merkle_ref, hash_fn, hash_block_fn);
    auto it2 = merkle;
    int nb_bogus = 0;
    int i = 0;
    for (auto it1 = merkle_ref; it1 != (merkle_ref + nb); it1++, it2++, i++) {
      if (! hash_type::same(*it1, *it2)) {
        nb_bogus++;
      }
    }
    if (nb_bogus > 0) {
      std::cout << "There were bogus results: nb_bogus = " << nb_bogus << std::endl;
    }
    free(merkle_ref);
  }
  return merkle;
}
  
template <int szb>
class byte_array {
public:
  static constexpr int szb1 = szb;
  
  byte_array() { }
  
  unsigned char contents[szb];
  
  unsigned char& operator[] (const int i) {
    assert(i >= 0);
    assert(i < szb);
    return contents[i];
  }
  
  unsigned char* begin() {
    return &contents[0];
  }
  
  unsigned char* end() {
    return begin() + szb1;
  }
  
  template <class T>
  T& cast_to() {
    return *((T*)&contents[0]);
  }
  
  static
  bool same(byte_array& lhs, byte_array& rhs) {
    bool s = true;
    for (int i = 0; i < szb; i++) {
      if (lhs.contents[i] != rhs.contents[i]) {
        s = false;
      }
    }
    return s;
  }
  
};
  
static constexpr int lg_half_kb = 9;
static constexpr int lg_one_kb = 10;
static constexpr int lg_ten_kb = 17;
static constexpr int lg_one_mb = 20;
static constexpr int lg_sixteen_mb = 24;
static constexpr int half_kb = 1 << lg_half_kb;
static constexpr int one_kb = 1 << lg_one_kb;
static constexpr int ten_kb = 1 << lg_ten_kb;
static constexpr int one_mb = 1 << lg_one_mb;
static constexpr int sixteen_mb = 1 << lg_sixteen_mb;

using half_kb_block = byte_array<half_kb>;
using one_kb_block = byte_array<one_kb>;
using ten_kb_block = byte_array<ten_kb>;
using one_mb_block = byte_array<one_mb>;
using sixteen_mb_block = byte_array<sixteen_mb>;
  
void initialize_block(int i, unsigned char* lo, unsigned char* hi) {
  int j = 0;
  for (auto it = lo; it != hi; it++, j++) {
    *it = (unsigned char)(prandgen::hashi(i | j) % 256);
  }
}
  
template <class Block_iterator>
void initialize_blocks(Block_iterator lo, Block_iterator hi) {
  auto nb_blocks = hi - lo;
  for (int i = 0; i < nb_blocks; i++) {
    auto lo2 = (unsigned char*)&(lo[i]);
    auto hi2 = lo2 + sizeof(typename std::iterator_traits<Block_iterator>::value_type);
    initialize_block(i, lo2, hi2);
  }
}
  
template <class Hash_policy>
void merkletree(pbbs::measured_type measure, const Hash_policy& hash_policy) {
  using hash_type = typename Hash_policy::digest_type;
  hash_type* merkle = nullptr;
  unsigned nb = 0;
  int nb_blocks = 1 << cmdline::parse<int>("nb_blocks_lg");
  cmdline::dispatcher d;
  d.add(std::to_string(lg_half_kb), [&] {

    std::vector<one_kb_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy, nb);
  });
  d.add(std::to_string(lg_one_kb), [&] {
    std::vector<one_kb_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy, nb);
  });
  d.add(std::to_string(lg_ten_kb), [&] {
    std::vector<ten_kb_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy, nb);
  });
  d.add(std::to_string(lg_one_mb), [&] {
    std::vector<one_mb_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy, nb);
  });
  d.add(std::to_string(lg_sixteen_mb), [&] {
    std::vector<sixteen_mb_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy, nb);
  });
  d.dispatch("block_szb_lg");
  if (cmdline::parse_or_default_bool("print", false)) {
    int i = 0;
    for (auto it = merkle; it != merkle + nb; it++, i++) {
      std::cout << "[" << i << ", " << it->template cast_to<unsigned int>() << "] ";
    }
    std::cout << std::endl;
  }
  free(merkle);
}
  
class pbbs32 {
public:
  
  using digest_type = byte_array<sizeof(unsigned int)>;
  
  static
  void hash_2(digest_type* lhs, digest_type* rhs, digest_type* dst) {
    auto h = lhs->cast_to<unsigned int>() | rhs->cast_to<unsigned int>();
    dst->cast_to<unsigned int>() = prandgen::hashi(h);
  }
  
  static
  void hash_range(const unsigned char* lo, const unsigned char* hi, digest_type* dst) {
    auto hash_fn = [&] (const unsigned char* src, size_t szb, unsigned char* dst) {
      assert(szb == digest_type::szb1);
      hash_2((digest_type*)dst, (digest_type*)src, (digest_type*)dst);
    };
    hash_range3(lo, hi, *dst, hash_fn);
  }
  
};

class sha256 {
public:
  
  using digest_type = byte_array<SHA256_DIGEST_LENGTH>;
  
  static
  void hash_2(digest_type* lhs, digest_type* rhs, digest_type* dst) {
    hash2(*lhs, *rhs, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA256(lo, n, dst);
    });
  }
  
  static
  void hash_range(const unsigned char* lo, const unsigned char* hi, digest_type* dst) {
    hash_range3(lo, hi, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA256(lo, n, dst);
    });
  }
  
};

class sha384 {
public:
  
  using digest_type = byte_array<SHA384_DIGEST_LENGTH>;
  
  static
  void hash_2(digest_type* lhs, digest_type* rhs, digest_type* dst) {
    hash2(*lhs, *rhs, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA384(lo, n, dst);
    });

  }
  
  static
  void hash_range(const unsigned char* lo, const unsigned char* hi, digest_type* dst) {
    hash_range3(lo, hi, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA384(lo, n, dst);
    });
  }
  
};

class sha512 {
public:
  
  using digest_type = byte_array<SHA512_DIGEST_LENGTH>;
  
  static
  void hash_2(digest_type* lhs, digest_type* rhs, digest_type* dst) {
    hash2(*lhs, *rhs, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA512(lo, n, dst);
    });
  }
  
  static
  void hash_range(const unsigned char* lo, const unsigned char* hi, digest_type* dst) {
    hash_range3(lo, hi, *dst, [] (const unsigned char* lo, size_t n, unsigned char* dst) {
      SHA512(lo, n, dst);
    });
  }
  
};
  
void merkletree(pbbs::measured_type measure) {
  cmdline::dispatcher d;
  d.add("pbbs32", [&] {
    merkletree(measure, pbbs32());
  });
  d.add("sha256", [&] {
    merkletree(measure, sha256());
  });
  d.add("sha384", [&] {
    merkletree(measure, sha384());
  });
  d.add("sha512", [&] {
    merkletree(measure, sha512());
  });
  d.dispatch("digest");
}

} // end namespace
} // end namespace

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measure) {
    pasl::pctl::merkletree(measure);
  });
  return 0;
}


