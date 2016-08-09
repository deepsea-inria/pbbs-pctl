/*!
 * \file granularity.cpp
 * \brief Example use of pctl granularity control
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include "bench.hpp"
#include "io.hpp"

/***********************************************************************/

namespace pasl {
namespace pctl {
  
int nb_occurrences_seq(char* lo, char* hi, char c) {
  int r = 0;
  for (; lo != hi; lo++) {
    if (*lo == c) {
      r++;
    }
  }
  return r;
}

namespace withoutgc {
  
  int nb_occurrences_rec(char* lo, char* hi, char c) {
    int r;
    auto n = hi - lo;
    if (n == 0) {
      r = 0;
    } else if (n == 1) {
      return *lo == c;
    } else {
      auto mid = lo + (n / 2);
      int r1, r2;
      granularity::primitive_fork2([&] {
        r1 = nb_occurrences_rec(lo, mid, c);
      }, [&] {
        r2 = nb_occurrences_rec(mid, hi, c);
      });
      r = r1 + r2;
    }
    return r;
  }
  
} // end namespace

namespace withgc {
  
  int threshold = 1;
  
  int nb_occurrences_rec(char* lo, char* hi, char c) {
    int r;
    auto n = hi - lo;
    if (n <= threshold) {
      r = nb_occurrences_seq(lo, hi, c);
    } else {
      auto mid = lo + (n / 2);
      int r1, r2;
      granularity::primitive_fork2([&] {
        r1 = nb_occurrences_rec(lo, mid, c);
      }, [&] {
        r2 = nb_occurrences_rec(mid, hi, c);
      });
      r = r1 + r2;
    }
    return r;
  }
  
} // end namespace

int max_nb_occurrences_seq(int* lo, int* hi, char* strings, char c) {
  int r = 0;
  for (; lo != hi; lo++) {
    int r1 = nb_occurrences_seq(strings + *lo, strings + *(lo + 1), c);
    r = std::max(r, r1);
  }
  return r;
}
  
namespace withoutgc {
  
  int max_nb_occurrences_rec(int* lo, int* hi, char* strings, char c) {
    int r;
    auto n = hi - lo;
    if (n == 0) {
      r = 0;
    } else if (n == 1) {
      r = withgc::nb_occurrences_rec(strings + *lo, strings + *(lo + 1), c);
    } else {
      auto mid = lo + (n / 2);
      int r1, r2;
      granularity::primitive_fork2([&] {
        r1 = max_nb_occurrences_rec(lo, mid, strings, c);
      }, [&] {
        r2 = max_nb_occurrences_rec(mid, hi, strings, c);
      });
      r = std::max(r1, r2);
    }
    return r;
  }
  
} // end namespace
  
namespace withgc {
  
  int max_nb_occurrences_rec(int* lo, int* hi, char* strings, char c) {
    int r;
    auto n = hi - lo;
    if (n == 0) {
      r = 0;
    } else {
      auto m = *hi - *lo;
      if (m <= threshold) {
        return max_nb_occurrences_seq(lo, hi, strings, c);
      } else {
        auto mid = std::lower_bound(lo, hi, m / 2);
        int r1, r2;
        granularity::primitive_fork2([&] {
          r1 = max_nb_occurrences_rec(lo, mid, strings, c);
        }, [&] {
          r2 = max_nb_occurrences_rec(mid, hi, strings, c);
        });
        r = std::max(r1, r2);
      }
    }
    return r;
  }
  
  int max_nb_occurrences_rec_alt(int* lo, int* hi, char* strings, char c) {
    int r;
    auto n = hi - lo;
    if (n <= threshold) {
      r = max_nb_occurrences_seq(lo, hi, strings, c);
    } else {
      auto m = *hi - *lo;
      if (m <= threshold) {
        return max_nb_occurrences_seq(lo, hi, strings, c);
      } else {
        auto mid = std::lower_bound(lo, hi, m / 2);
        int r1, r2;
        granularity::primitive_fork2([&] {
          r1 = max_nb_occurrences_rec_alt(lo, mid, strings, c);
        }, [&] {
          r2 = max_nb_occurrences_rec_alt(mid, hi, strings, c);
        });
        r = std::max(r1, r2);
      }
    }
    return r;
  }
  
} // end namespace
  
}
}

/*---------------------------------------------------------------------*/

void write_random_chars(char* lo, char* hi) {
  for (; lo != hi; lo++) {
    *lo = rand() % 256;
  }
}

// returns segmented array to represent k length-m random strings
std::tuple<int*, int*, char*> create_random_string_array(int k, int m) {
  int n = m * k;
  char* strings = (char*)malloc(sizeof(char) * n);
  int* lo = (int*)malloc(sizeof(int) * (k + 1));
  int* hi = lo + (k + 1);
  for (int i = 0; i < k; i++) {
    lo[i] = i * m;
  }
  return std::make_tuple(lo, hi, strings);
}

template <class Untrusted>
bool check_nb_occurrences(char* lo, char* hi, char c, const Untrusted& u) {
  return pasl::pctl::nb_occurrences_seq(lo, hi, c) == u(lo, hi, c);
}

void check_nb_occurrences(int n) {
  if (n == 0) {
    return;
  }
  char* string = (char*)malloc(sizeof(char) * n);
  auto lo = string;
  auto hi = string + n;
  write_random_chars(lo, hi);
  auto c = *lo;
  assert(check_nb_occurrences(lo, hi, c, [&] (char* lo, char* hi, char c) {
    return pasl::pctl::withoutgc::nb_occurrences_rec(lo, hi, c);
  }));
  assert(check_nb_occurrences(lo, hi, c, [&] (char* lo, char* hi, char c) {
    return pasl::pctl::withgc::nb_occurrences_rec(lo, hi, c);
  }));
}

template <class Untrusted>
bool check_max_nb_occurrences(int* lo, int* hi, char* strings, char c, const Untrusted& u) {
  return pasl::pctl::max_nb_occurrences_seq(lo, hi, strings, c) == u(lo, hi, strings, c);
}

void check_max_nb_occurrences(int k, int m) {
  int n = m * k;
  if (n == 0) {
    return;
  }
  auto sa = create_random_string_array(k, m);
  int* lo = std::get<0>(sa);
  int* hi = std::get<1>(sa);
  char* strings = std::get<2>(sa);
  auto c = strings[0];
  check_max_nb_occurrences(lo, hi, strings, c, [&] (int* lo, int* hi, char* strings, char c) {
    return pasl::pctl::withoutgc::max_nb_occurrences_rec(lo, hi, strings, c);
  });
  check_max_nb_occurrences(lo, hi, strings, c, [&] (int* lo, int* hi, char* strings, char c) {
    return pasl::pctl::withgc::max_nb_occurrences_rec(lo, hi, strings, c);
  });
}

namespace cmdline = deepsea::cmdline;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    pasl::pctl::withgc::threshold =
      cmdline::parse_or_default("threshold", pasl::pctl::withgc::threshold);
    int n = cmdline::parse_or_default("n", 1024);
    for (int i = 1; i < n; i++) {
      check_nb_occurrences(n);
    }
    for (int k = 1; k < n; k++) {
      for (int m = 1; m < n; m++) {
        check_max_nb_occurrences(k, m);
      }
    }
  });
  return 0;
}

/***********************************************************************/
