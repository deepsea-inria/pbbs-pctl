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

template <class Unsigned>
Unsigned round_up_to_power_of_2(Unsigned v) {
  static_assert(std::is_unsigned<Unsigned>::value, "Only works for unsigned types");
  v--;
  for (int i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) {
    v |= v >> i;
  }
  return ++v;
}

namespace pasl {
namespace pctl {

using namespace granularity;
  
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
void merkletree(Block_iterator lo, Block_iterator hi, Hash* merkle_lo,
                const Hash_fn& hash_fn,
                const Hash_block_fn& hash_block_fn) {
  auto row_lo = round_up_to_power_of_2((unsigned)(hi - lo));
  int i = 0;
  for (auto it = lo; it != hi; it++, i++) {
    hash_block_fn(it, merkle_lo + row_lo + i);
  }
  assert(i == row_lo);
  for (row_lo = parent(row_lo); row_lo != 0; row_lo = parent(row_lo)) {
    for (auto i = row_lo; i < 2 * row_lo; i++) {
      size_t l = left_child(i);
      size_t r = right_child(i);
      hash_fn(merkle_lo + l, merkle_lo + r, merkle_lo + i);
    }
  }
}
  
template <class Block_iterator, class Hash_policy>
typename Hash_policy::digest_type* merkletree(pbbs::measured_type measure,
                                              Block_iterator lo, Block_iterator hi,
                                              const Hash_policy& hash_policy) {
  using hash_type = typename Hash_policy::digest_type;
  auto n = round_up_to_power_of_2((unsigned)(hi - lo));
  hash_type* merkle = malloc_array<hash_type>(2 * n);
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
      merkletree(lo, hi, merkle, hash_fn, hash_block_fn);
    });
  });
  d.dispatch("algorithm");
  return merkle;
}
  
template <int szb>
class byte_array {
public:
  unsigned char contents[szb];
  unsigned char& operator[] (const int i) {
    assert(i >= 0);
    assert(i < szb);
    return contents[i];
  }
};
  
static constexpr int one_kb = 1 << 10;
static constexpr int ten_kb = 1 << 17;
static constexpr int one_mb = 1 << 20;

using small_block = byte_array<one_kb>;
using medium_block = byte_array<ten_kb>;
using large_block = byte_array<one_mb>;
  
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
  int block_szb = cmdline::parse<int>("block_szb");
  int nb_blocks = 1 << cmdline::parse<int>("nb_blocks_lg");
  if (block_szb == one_kb) {
    std::vector<small_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy);
  } else if (block_szb == ten_kb) {
    std::vector<medium_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy);
  } else if (block_szb == one_mb) {
    std::vector<large_block> blocks(nb_blocks);
    initialize_blocks(blocks.begin(), blocks.end());
    merkle = merkletree(measure, blocks.begin(), blocks.end(), hash_policy);
  } else {
    std::cerr << "Bogus block_szb " << block_szb << std::endl;
    exit(0);
  }
  free(merkle);
}

class sha256 {
public:
  
  using digest_type = byte_array<SHA256_DIGEST_LENGTH>;
  
  static
  void hash_2(digest_type* lhs, digest_type* rhs, digest_type* dst) {
    digest_type tmp;
    std::copy((char*)lhs, (char*)(lhs + 1), (char*)&tmp);
    for (int i = 0; i < sizeof(digest_type); i++) {
      (*dst)[i] = tmp[i] | (*rhs)[i];
    }
    SHA1((const unsigned char*)(&tmp), sizeof(digest_type), (unsigned char*)dst);
  }
  
  static
  void hash_range(const unsigned char* lo, const unsigned char* hi, digest_type* dst) {
    SHA1(lo, hi - lo, (unsigned char*)dst);
  }
  
};
  
void merkletree(pbbs::measured_type measure) {
  cmdline::dispatcher d;
  d.add("sha256", [&] {
    merkletree(measure, sha256());
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


