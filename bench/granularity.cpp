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

/*---------------------------------------------------------------------*/

void write_random_chars(char* lo, char* hi) {
  for (; lo != hi; lo++) {
    *lo = rand() % 256;
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
  d.dispatch("algorithm");
  if (result < 0) {
    std::cerr << "bogus result" << std::endl;
  }
}
  
template <class Item>
void benchmark(pbbs::measured_type measure) {
  int n = cmdline::parse<int>("n");
  Item* data = create_random_array<Item>(n);
  auto lo = data;
  auto hi = lo + n;
  auto c = *lo;
  if (! cmdline::parse<bool>("use_hash")) {
    assert(sizeof(Item) == sizeof(char));
    auto p = [c] (Item d) {
      return d == c;
    };
    benchmark(lo, hi, p, measure);
  } else {
    auto h = hash(c);
    auto p = [h] (Item& d) {
      return h == hash(d);
    };
    benchmark(lo, hi, p, measure);
  }
  free(data);
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
  
static constexpr
int szb1 = 4 * 256;
static constexpr
int szb2 = 4 * 512;
  
void determine_item_type(pbbs::measured_type measure) {
  with_gc::threshold = cmdline::parse_or_default("threshold", with_gc::threshold);
  int item_szb = cmdline::parse<int>("item_szb");
  if (item_szb == 1) {
    using_item_type<char>(measure);
  } else if (item_szb == szb1) {
    using_item_type<char[szb1]>(measure);
  } else if (item_szb == szb2) {
    using_item_type<char[szb2]>(measure);
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
