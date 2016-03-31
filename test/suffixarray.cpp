/*!
 * \file scan.cpp
 * \brief Quickcheck for scan
 * \date 2015
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <string>

#include "test.hpp"
#include "prandgen.hpp"
#include "suffixarray.hpp"
#include "trigrams.hpp"

/***********************************************************************/

namespace pasl {
namespace pctl {

/*---------------------------------------------------------------------*/
/* Quickcheck IO */

template <class Container>
std::ostream& operator<<(std::ostream& out, const container_wrapper<Container>& c) {
  out << c.c;
  return out;
}

/*---------------------------------------------------------------------*/
/* Quickcheck generators */
  
using value_type = unsigned char;
  
void generate(size_t nb, parray<value_type>& str) {
  nb = nb * 100;
  std::cerr << "Size: " << nb << std::endl;
  str.resize(nb + 1, 0);
  int mode = quickcheck::generateInRange(0, 3);
  if (mode == 0) {
    for (int i = 0; i < nb; i++) {
      int x = quickcheck::generateInRange(0, 26) + 'a';
      str[i] = (value_type)x;
    }
  } else {
    for (int i = 0; i < nb; i++) {
      str[i] = 'a';
    }
  }
}
  
void generate(size_t nb, container_wrapper<parray<value_type>>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
typedef unsigned char uchar;

bool str_less_bounded (uchar* s1c, uchar* s2c, intT n) {
  uchar* s1 = s1c;
  uchar* s2 = s2c;
  while (*s1 && *s1 == *s2) {
    if (n-- < 0) {
      return 1;
    }
    s1++;
    s2++;
  };
  return (*s1 < *s2);
}

bool is_permutation(intT* suffixes, intT n) {
  parray<bool> seen(n, false);

  parallel_for((intT)0, n, [&] (intT i) {
    seen[suffixes[i]] = true;
  });
  bool nseen = reduce(seen.cbegin(), seen.cend(), true, [&] (bool x, bool y) {
    return x & y;
  });
  return nseen;
}

bool is_sorted(intT* suffixes, uchar* s, intT n) {
  int check_len = 10;
  intT error = n;

  intT p = 239;
  parray<long long> hash(n + 1);
  parray<long long> pow(n + 1);
  hash[0] = 0;
  pow[0] = 1;
  for (int i = 1; i <= n; i++) {
    hash[i] = hash[i - 1] * p + s[i - 1];
    pow[i] = pow[i - 1] * p;
  }

  parallel_for((intT)0, n - 1, [&] (intT i) {
    uchar* a = s + suffixes[i];
    int la = n - suffixes[i];
    uchar* b = s + suffixes[i + 1];
    int lb = n - suffixes[i + 1];
    int l = -1;
    int r = std::min(la, lb);
    while (l < r - 1) {
      int m = (l + r) >> 1;
      long long ha = hash[suffixes[i] + m + 1] - hash[suffixes[i]] * pow[m + 1];
      long long hb = hash[suffixes[i + 1] + m + 1] - hash[suffixes[i + 1]] * pow[m + 1];
      if (ha == hb) {
        l = m;
      } else {
        r = m;
      }
    }
//    std::cerr << la << " " << lb << " " << r << std::endl;

    if (((r != la && r != lb && a[r] > b[r]) || r == lb) || (!str_less_bounded(s + suffixes[i], s + suffixes[i + 1], check_len))) {
      //cout.write((char*) s+SA[i],checkLen); cout << endl;
      //cout.write((char*) s+SA[i+1],min(checkLen,n-SA[i+1]));cout << endl;
      utils::writeMin(&error,i);
    }
  });
  if (error != n) {
    cout << "Suffix Array Check: not sorted at i = " << error+1 << endl;
    return false;
  }
  return true;
}

using parray_wrapper = container_wrapper<parray<value_type>>;

class suffixarray_property : public quickcheck::Property<parray_wrapper> {
public:
  
  bool holdsFor(const parray_wrapper& _in) {
    parray_wrapper in(_in);
    intT n = (intT)in.c.size() - 1;
    parray<intT> suffixes = suffix_array(in.c.begin(), n);

//    std::cerr << suffixes << std::endl;

    if (suffixes.size() != n) {
      return false;
    }
    if (!is_permutation(suffixes.begin(), n)) {
      return false;
    }
    if (!is_sorted(suffixes.begin(), in.c.begin(), n)) {
      return false;
    }
    return true;
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::suffixarray_property>(nb_tests, "suffixarray is correct");
  });
  return 0;
}

/***********************************************************************/
