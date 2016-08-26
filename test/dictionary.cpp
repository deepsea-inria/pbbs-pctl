/*!
 * \file dictionary.cpp
 * \brief Quickcheck for dictionary
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <set>

#include "test.hpp"
#include "prandgen.hpp"
#include "sequenceio.hpp"
#include "deterministichash.hpp"

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
  
template <class Item>
std::ostream& operator<<(std::ostream& out, const std::set<Item>& xs) {
  out << "{ ";
  for (auto it = xs.cbegin(); it != xs.cend(); it++) {
    out << (*it);
    auto it2 = it;
    it2++;
    if (it2 != xs.cend())
      out << ", ";
  }
  out << " }";
  return out;
}

/*---------------------------------------------------------------------*/
/* Quickcheck generators */

using value_type = int;

template <class Type>
class test_case {
public:
  parray<std::pair<Type, value_type>*> insert;
  parray<std::pair<Type, value_type>*> remove;
  parray<std::pair<Type, value_type>*> find;
};

template <class Item>
std::ostream& operator<<(std::ostream& out, const test_case<Item>& test) {
  out << "insert: {";
  bool first = true;
  for (auto x : test.insert) {
    if (!first) {
      out << ", ";
    }
    out << "(" << x->first << ", " << x->second << ")";
    first = false;
  }
  out << "}\n";

  out << "remove: {";
  first = true;
  for (auto x : test.remove) {
    if (!first) {
      out << ", ";
    }
    out << "(" << x->first << ", " << x->second << ")";
    first = false;
  }
  out << "}\n";

  out << "find: {";
  first = true;
  for (auto x : test.find) {
    if (!first) {
      out << ", ";
    }
    out << "(" << x->first << ", " << x->second << ")";
    first = false;
  }
  out << "}";
}

const int MAX_INT = 1000000000;

void generate(size_t nb, test_case<value_type>& dst) {
/*  if (quickcheck::generateInRange(0, 4) == 0) {
    nb = nb * 10000;
  }*/
  nb = nb * 100;
  std::cerr << "Size: " << nb << "\n";

  int li = 2 * nb;
  int ld = nb / 2;
  int lf = nb;

  dst.insert.resize(li);
  dst.remove.resize(ld);
  dst.find.resize(lf);

  for (int i = 0; i < nb; i++) {
    int p = i;
    if (quickcheck::generateInRange(0, 2) == 1) {
      p = 2 * nb - 1 - i;
    }
    value_type value = quickcheck::generateInRange(0, MAX_INT);
    dst.insert[p] = new pair<value_type, value_type>(value, 1);
    dst.insert[2 * nb - 1 - p] = new pair<value_type, value_type>(value, 2);
  }
  for (int i = 0; i < ld; i++) {
    dst.remove[i] = dst.insert[i];//quickcheck::generateInRange(0, li - 1)];
  }
  for (int i = 0; i < lf; i++) {
    dst.find[i] = dst.insert[i];
  }
}

void generate(size_t nb, container_wrapper<test_case<value_type>>& c) {
  generate(nb, c.c);
}


/*---------------------------------------------------------------------*/
/* Quickcheck properties */
  
using test_case_wrapper = container_wrapper<test_case<value_type>>;

class prop : public quickcheck::Property<test_case_wrapper> {
public:
  
  bool holdsFor(const test_case_wrapper& _in) {
    const test_case<value_type>& test = _in.c;

//    std::cerr << "Before call\n";
    parray<std::pair<value_type, value_type>*> result = run_dict(test.insert, test.remove, test.find, hash_pair<hash_int<value_type>, value_type>(hash_int<value_type>()));
//    std::cerr << "After call\n";

//    std::cerr << "Before check\n";
    std::map<value_type, value_type> seq_result;
    for (auto x : test.insert) {
      if (seq_result.count(x->first) == 0) {
        seq_result[x->first] = x->second;
      } else {
        seq_result[x->first] = std::max(seq_result[x->first], x->second);
      }
    }
    for (auto x : test.remove) {
      seq_result.erase(x->first);
    }
    for (int i = 0; i < test.find.size(); i++) {
      value_type x = test.find[i]->first;
      if (seq_result.count(x) == 0) {
        if (result[i] != NULL) {
          std::cout << "Value is removed, but not from dictionary: " << result[i]->first << " " << result[i]->second << "\n";
          return false;
        }
      } else {
        if (result[i] == NULL || (result[i]->second != seq_result[x])) {
          std::cout << "Overwritten value isn't right: " << result[i]->second << "\n";
          return false;
        }
      }
    }
//    std::cerr << "After check\n";

    return true;
  }
  
};

} // end namespace
} // end namespace

/*---------------------------------------------------------------------*/

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] {
    int nb_tests = pasl::util::cmdline::parse_or_default_int("n", 1000);
    checkit<pasl::pctl::prop>(nb_tests, "deterministic hash is correct");
  });
  return 0;
}

/***********************************************************************/
