/*!
 * \file granularity.cpp
 * \brief Example use of pctl granularity control
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "bench.hpp"
#include "prandgen.hpp"

/***********************************************************************/

namespace cmdline = deepsea::cmdline;

static constexpr
int szb1 = 4 * 256;
static constexpr
int szb2 = 2*szb1;
static constexpr
int szb3 = 1 << 20;

template <int szb>
class bytes {
public:
  char b[szb];
};

template <class Item>
using segment_descriptor = std::pair<Item*, int>; // pair of item start pointer, offset

namespace pasl {
namespace pctl {
  
using namespace granularity;
  
template <class Item, class Predicate>
int nb_occurrences_seq(Item* lo, Item* hi, const Predicate& p) {
  int r = 0;
  for (; lo != hi; lo++) {
    if (p(*lo)) {
      r++;
    }
  }
  return r;
}

namespace without_gc {
  
  template <class Item, class Predicate>
  int nb_occurrences_rec(Item* lo, Item* hi, const Predicate& p) {
    int r;
    auto n = hi - lo;
    if (n == 0) {
      r = 0;
    } else if (n == 1) {
      return p(*lo) ? 1 : 0;
    } else {
      auto mid = lo + (n / 2);
      int r1, r2;
      primitive_fork2([&] {
        r1 = nb_occurrences_rec(lo, mid, p);
      }, [&] {
        r2 = nb_occurrences_rec(mid, hi, p);
      });
      r = r1 + r2;
    }
    return r;
  }
  
} // end namespace
  
namespace with_gc {
  
  int threshold = 2;
  
  template <class Item, class Predicate>
  int nb_occurrences_rec(Item* lo, Item* hi, const Predicate& p) {
    int r;
    auto n = hi - lo;
    if (n <= threshold) {
      r = nb_occurrences_seq(lo, hi, p);
    } else {
      auto mid = lo + (n / 2);
      int r1, r2;
      primitive_fork2([&] {
        r1 = nb_occurrences_rec(lo, mid, p);
      }, [&] {
        r2 = nb_occurrences_rec(mid, hi, p);
      });
      r = r1 + r2;
    }
    return r;
  }
  
} // end namespace

namespace with_oracle_guided {
  
  template <class Item, class Predicate>
  class nb_occurrences_rec_contr {
  public:
    static controller_type contr;
  };

  template <class Item, class Predicate>
  controller_type nb_occurrences_rec_contr<Item, Predicate>::contr("nb_occurrences_rec" +
                                                                   sota<Item>() + sota<Predicate>());
  
  template <class Item, class Predicate>
  int nb_occurrences_rec(Item* lo, Item* hi, const Predicate& p) {
    using controller_type = nb_occurrences_rec_contr<Item, Predicate>;
    int r;
    auto n = hi - lo;
    par::cstmt(controller_type::contr, [&] { return n; }, [&] {
      if (n == 0) {
        r = 0;
      } else if (n == 1 && p(*lo)) {
        r = 1;
      } else if (n == 1) {
        r = 0;
      } else {
        auto mid = lo + (n / 2);
        int r1, r2;
        primitive_fork2([&] {
          r1 = nb_occurrences_rec(lo, mid, p);
        }, [&] {
          r2 = nb_occurrences_rec(mid, hi, p);
        });
        r = r1 + r2;
      }
    });
    return r;
  }
  
} // end namespace
  
namespace parallel_with_oracle_guided_and_seq_alt_body {
  
  template <class Item, class Predicate>
  class nb_occurrences_rec_contr {
  public:
    static controller_type contr;
  };
  
  template <class Item, class Predicate>
  controller_type nb_occurrences_rec_contr<Item, Predicate>::contr("nb_occurrences_rec" +
                                                                   sota<Item>() + sota<Predicate>());
  
  template <class Item, class Predicate>
  int nb_occurrences_rec(Item* lo, Item* hi, const Predicate& p) {
    using controller_type = nb_occurrences_rec_contr<Item, Predicate>;
    int r;
    auto n = hi - lo;
    par::cstmt(controller_type::contr, [&] { return n; }, [&] {
      if (n == 0) {
        r = 0;
      } else if (n == 1 && p(*lo)) {
        r = 1;
      } else if (n == 1) {
        r = 0;
      } else {
        auto mid = lo + (n / 2);
        int r1, r2;
        primitive_fork2([&] {
          r1 = nb_occurrences_rec(lo, mid, p);
        }, [&] {
          r2 = nb_occurrences_rec(mid, hi, p);
        });
        r = r1 + r2;
      }
    }, [&] {
      r = nb_occurrences_seq(lo, hi, p);
    });
    return r;
  }
  
} // end namespace
  
namespace parallel_with_level1_reduce {
  
  template <class Item, class Predicate>
  int nb_occurrences(Item* lo, Item* hi, const Predicate& p) {
    return level1::reduce(lo, hi, 0, [&] (int x, int y) {
      return x + y;
    }, [&] (Item& i) {
      return (p(i)) ? 1 : 0;
    });
  }
  
} // end namespace
  
template <class Pointer, class Weight, class Predicate>
int nb_occurrences_seq(int lo, int hi, Pointer d, const Weight& w, const Predicate& p) {
  int r = 0;
  for (int i = lo; i < hi; i++) {
    auto v = d[i];
    r += nb_occurrences_seq(v.first, v.second, p);
  }
  return r;
}
  
namespace nested_parallel_with_gc {
  
  template <class Pointer, class Weight, class Predicate>
  int nb_occurrences(int lo, int hi, Pointer d, const Weight& w, const Predicate& p) {
    int r = 0;
    if (w(lo, hi, d) <= with_gc::threshold) {
      r = nb_occurrences_seq(lo, hi, d, w, p);
    } else if ((hi - lo) <= 2) {
      for (int i = lo; i < hi; i++) {
        auto v = d[i];
        r += with_gc::nb_occurrences_rec(v.first, v.second, p);
      }
    } else {
      int mid = (lo + hi) / 2;
      int r1, r2;
      primitive_fork2([&] {
        r1 = nb_occurrences(lo, mid, d, w, p);
      }, [&] {
        r2 = nb_occurrences(mid, hi, d, w, p);
      });
      r = r1 + r2;
    }
    return r;
  }
  
} // end namespace
  
namespace nested_parallel_with_level2 {
  
  int threshold;
  
  template <class Pointer, class Weight, class Predicate>
  int nb_occurrences(int lo, int hi, Pointer d, const Weight& w, const Predicate& p) {
    using segment_descriptor_type = segment_descriptor<typename Pointer::value_type>;
    segment_descriptor_type* sd = d.segdes;
    auto lo2 = sd + lo;
    return level2::reduce(lo2, sd + hi, 0, [&] (int x, int y) {
      return x + y;
    }, [&] (segment_descriptor_type* lo3, segment_descriptor_type* hi3) {
      return w(lo3 - lo2, hi3 - lo2, d);
    }, [&] (long i, segment_descriptor_type&) {
      auto v = d[i];
      return with_oracle_guided::nb_occurrences_rec(v.first, v.second, p);
    }, [&] (segment_descriptor_type* lo3, segment_descriptor_type* hi3) {
      auto lo4 = lo3 - lo2;
      auto hi4 = hi3 - lo2;
      return nb_occurrences_seq(lo4, hi4, d, w, p);
    });
  }
  
} // end namespace

/*---------------------------------------------------------------------*/

void write_random_chars(char* lo, char* hi) {
  for (int i = 0; lo != hi; lo++, i++) {
    union {
      char* p;
      uint64_t v;
    } x;
    x.p = lo;
    *lo = (char)prandgen::hashi((unsigned int)x.v);
  }
}
  
template <class Item>
void write_random_data(Item* lo, Item* hi) {
  for (auto it = lo; it != hi; it++) {
    auto lo2 = (char*)it;
    auto hi2 = lo2 + sizeof(Item);
    write_random_chars(lo2, hi2);
  }
}
  
template <class Item>
Item* create_random_array(int n) {
  Item* data = (Item*)malloc(sizeof(Item) * n);
  auto lo = data;
  auto hi = data + n;
  write_random_data(lo, hi);
  return data;
}

template <class Item>
std::pair<segment_descriptor<Item>*, Item*> create_nested_array(int n) {
  std::pair<segment_descriptor<Item>*, Item*> r;
  auto rows = (segment_descriptor<Item>*)malloc(sizeof(segment_descriptor<Item>) * (n + 1));
  auto bits = create_random_array<Item>(n * (n + 1) / 2);
  int o = 0;
  Item* p = bits;
  for (int i = 0; i < n + 1; i++) {
    new (&rows[i]) segment_descriptor<Item>(p, o);
    p += i;
    o += i;
  }
  r.first = rows;
  r.second = bits;
  return r;
}

template <class Item>
void check_nb_occurrences(int n) {
  if (n == 0) {
    return;
  }
  Item* data = create_random_array<Item>(n);
  auto lo = data;
  auto hi = data + n;
  auto c = *lo;
  auto p = [c] (Item d) {
    return d == c;
  };
  int nb = nb_occurrences_seq(lo, hi, p);
  assert(without_gc::nb_occurrences_rec(lo, hi, p) == nb);
  assert(with_gc::nb_occurrences_rec(lo, hi, p) == nb);
  free(data);
}

template <class Item>
unsigned int hash(Item& x) {
  unsigned int h = 0;
  assert(sizeof(Item) % sizeof(unsigned int) == 0);
  auto lo = (unsigned int*)&x;
  auto hi = lo + (sizeof(Item) / sizeof(unsigned int));
  for (auto it = lo; it != hi; it++) {
    h = prandgen::hashu(*it + h);
  }
  return h;
}
  
template <class Item, class Predicate>
void benchmark(Item* lo, Item* hi, const Predicate& p, pbbs::measured_type measure) {
  int result;
  cmdline::dispatcher d;
  d.add("sequential", [&] {
    measure([&] {
      result = nb_occurrences_seq(lo, hi, p);
    });
  });
  d.add("parallel_without_gc", [&] {
    measure([&] {
      result = without_gc::nb_occurrences_rec(lo, hi, p);
    });
  });
  d.add("parallel_with_gc", [&] {
    measure([&] {
      result = with_gc::nb_occurrences_rec(lo, hi, p);
    });
  });
  d.add("parallel_with_oracle_guided", [&] {
    measure([&] {
      result = with_oracle_guided::nb_occurrences_rec(lo, hi, p);
    });
  });
  d.add("parallel_with_oracle_guided_and_seq_alt_body", [&] {
    measure([&] {
      result = parallel_with_oracle_guided_and_seq_alt_body::nb_occurrences_rec(lo, hi, p);
    });
  });
  d.add("parallel_with_level1_reduce", [&] {
    measure([&] {
      result = parallel_with_level1_reduce::nb_occurrences(lo, hi, p);
    });
  });
  d.dispatch("algorithm");
  if (result < 0) {
    std::cerr << "bogus result" << std::endl;
  }
  assert(result == nb_occurrences_seq(lo, hi, p));
}
  
template <class Pointer, class Weight, class Predicate>
void benchmark(int lo, int hi, Pointer d, const Weight& w, const Predicate& p,
               pbbs::measured_type measure) {
  int result;
  cmdline::dispatcher dp;
  dp.add("nested_parallel_with_gc", [&] {
    measure([&] {
      result = nested_parallel_with_gc::nb_occurrences(lo, hi, d, w, p);
    });
  });
  dp.add("nested_parallel_with_level2", [&] {
    measure([&] {
      result = nested_parallel_with_level2::nb_occurrences(lo, hi, d, w, p);
    });
  });
  dp.dispatch("algorithm");
  assert(result == nb_occurrences_seq(lo, hi, d, w, p));
}

bool is_nested() {
  std::string s = cmdline::parse_or_default<std::string>("algorithm", "");
  return s.find("nested") != std::string::npos;
}
  
template <class Item>
class indexer {
public:
  
  using value_type = Item;
  
  segment_descriptor<Item>* segdes;
  
  indexer(segment_descriptor<Item>* segdes) : segdes(segdes) { }
  
  std::pair<Item*, Item*> operator[] (const int i) {
    auto p = segdes[i];
    auto lo = p.first;
    auto sz = segdes[i + 1].second - p.second;
    auto hi = lo + sz;
    return std::make_pair(lo, hi);
  }

};

template <int szb>
inline
bool operator==(const bytes<szb>& lhs, const bytes<szb>& rhs) {
  assert(false);
  return false;
}
  
template <class Item>
void benchmark(pbbs::measured_type measure) {
  int n = cmdline::parse<int>("n");
  int nb_hash_iters = cmdline::parse_or_default("nb_hash_iters", 0);
  if (! is_nested()) {
    Item* data = create_random_array<Item>(n);
    auto lo = data;
    auto hi = lo + n;
    auto c = *lo;
    if (! cmdline::parse<bool>("use_hash")) {
      assert(sizeof(Item) == sizeof(char));
      auto p = [c] (Item& d) {
        return d == c;
      };
      benchmark(lo, hi, p, measure);
    } else if (nb_hash_iters == 0) {
      auto h = hash(c);
      auto p = [h] (Item& d) {
        return h == hash(d);
      };
      benchmark(lo, hi, p, measure);
    } else {
      auto h = hash(c);
      auto p = [h,nb_hash_iters] (Item& d) {
        unsigned int hh = hash(d);
        for (int i = 0; i < nb_hash_iters; i++) {
          hh = hash(hh);
        }
        return h == hh;
      };
      benchmark(lo, hi, p, measure);
    } 
    free(data);
  } else {
    int lo = 0;
    int hi = n;
    std::pair<segment_descriptor<Item>*, Item*> data = create_nested_array<Item>(n);
    indexer<Item> idxr(data.first);
    auto c = *data.second;
    auto w = [&] (int lo, int hi, indexer<Item> idxr){
      auto d = idxr.segdes;
      return d[hi].second - d[lo].second;
    };
    if (! cmdline::parse<bool>("use_hash")) {
      assert(sizeof(Item) == sizeof(char));
      auto p = [c] (Item& d) {
        return d == c;
      };
      benchmark(lo, hi, idxr, w, p, measure);
    } else if (nb_hash_iters == 0) {
      auto h = hash(c);
      auto p = [h] (Item& d) {
        return h == hash(d);
      };
      benchmark(lo, hi, idxr, w, p, measure);
    } else {
      auto h = hash(c);
      auto p = [h,nb_hash_iters] (Item& d) {
        unsigned int hh = hash(d);
        for (int i = 0; i < nb_hash_iters; i++) {
          hh = hash(hh);
        }
        return h == hh;
      };
      benchmark(lo, hi, idxr, w, p, measure);
    }
    free(data.first);
    free(data.second);
  }
}
  
template <class Item>
void test() {
  int n = cmdline::parse_or_default("n", 1024);
  for (int i = 1; i < n; i++) {
    pasl::pctl::check_nb_occurrences<char>(n);
  }
}
  
template <class Item>
void using_item_type(pbbs::measured_type measure) {
  cmdline::dispatcher d;
  d.add("benchmark", [&] {
    benchmark<Item>(measure);
  });
  d.add("test", [&] {
    test<Item>();
  });
  d.dispatch_or_default("action", "benchmark");
}
  
void determine_item_type(pbbs::measured_type measure) {
  with_gc::threshold = cmdline::parse_or_default("threshold", with_gc::threshold);
  int item_szb = cmdline::parse<int>("item_szb");
  if (item_szb == 1) {
    using_item_type<char>(measure);
  } else if (item_szb == szb1) {
    using_item_type<bytes<szb1>>(measure);
  } else if (item_szb == szb2) {
    using_item_type<bytes<szb2>>(measure);
  } else if (item_szb == szb3) {
    using_item_type<bytes<szb3>>(measure);
  } else {
    std::cerr << "bogus item_szb " <<  item_szb << std::endl;
    exit(0);
  }
}
  
} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measure) {
    pasl::pctl::determine_item_type(measure);
  });
  return 0;
}

/***********************************************************************/
