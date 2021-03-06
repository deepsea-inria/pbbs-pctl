/*!
 * \file deterministichash_bench.cpp
 * \brief Benchmarking script for parallel hash table
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <math.h>
#include <functional>
#include <stdlib.h>
#include "bench.hpp"
#include "deterministichash.hpp"
#include "loaders.hpp"
#include "deterministicHash.h"
#undef parallel_for
/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

template <class Item>
void pbbs_pctl_call(pbbs::measured_type measured, parray<Item>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    measured([&] {
      pbbs::removeDuplicates(pbbs::_seq<Item>(&x[0], x.size()));
    });
  } else {
    measured([&] {
      pasl::pctl::remove_duplicates(x);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      deepsea::cmdline::dispatcher d;
      d.add("array_int", [&] {
        parray<int> x = pasl::pctl::io::load<parray<int>>(infile);
        pbbs_pctl_call(measured, x);
      });
      d.add("array_string", [&] {
        parray<char*> x = pasl::pctl::io::load<parray<char*>>(infile);
        pbbs_pctl_call(measured, x);
      });
      d.add("array_pair_string_int", [&] {
        parray<std::pair<char*, int>*> x = pasl::pctl::io::load<parray<std::pair<char*, int>*>>(infile);
        pbbs_pctl_call(measured, x);
      });
      d.dispatch("type");
      return;
    }
    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    int m = deepsea::cmdline::parse_or_default_int("m", 1000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = deepsea::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
    system("mkdir tests");
    if (test == 0) {
      parray<int> a;
      if (!files) {
        a = pasl::pctl::io::load_random_seq<int>(std::string("tests/random_seq_int_") + std::to_string(n), n, reload);
      } else {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_seq_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int"), 10000000, reload);
      }
/*      std::cerr << "CHECK!\n";

//      parray<int> f = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_seq_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int"), 10000000, true);
      parray<int> f = pasl::pctl::io::load_random_seq<int>(std::string("tests/random_seq_int_10000000"), 10000000, true);
      parray<int> s = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_seq_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int"), 10000000, false);

      if (s.size() != f.size()) {
        std::cerr << "Not equal\n";
        return;
      }
      for (int i = 0; i < s.size(); i++) {
        if (f[i] != s[i]) {
          std::cerr << "Not equal " << i << "\n";
          std::cerr << f[i] << " " << s[i] << "\n";
          return;
        } 
      }*/

      pbbs_pctl_call(measured, a);
    } else if (test == 1) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_bounded_seq_txt_10000000_100000"), path_to_data + std::string("randomSeq_10M_100K_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq(std::string("tests/random_bounded_seq_") + std::to_string(n) + "_" + std::to_string(m), n, m, reload);
      }
      pbbs_pctl_call(measured, a);
    } else if (test == 2) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_exp_dist_seq_int_txt_10000000"), path_to_data + std::string("exptSeq_10M_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<int>(std::string("tests/random_exp_dist_seq_int_") + std::to_string(n), n, reload);
      }
      pbbs_pctl_call(measured, a);
    } else if (test == 3) {
      parray<char*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<char*>(std::string("tests/trigram_words_txt_10000000"), path_to_data + std::string("trigramSeq_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_trigram_words(std::string("tests/trigram_words_") + std::to_string(n), n, reload);
      }
      pbbs_pctl_call(measured, a);
      pasl::pctl::parallel_for(0, n, [&] (int i) {
        delete [] a[i];
      });
    } else if (test == 4) {
      parray<std::pair<char*, int>*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<std::pair<char*, int>*>(std::string("tests/trigram_words_int_txt_10000000"), path_to_data + std::string("trigramSeq_10M_pair_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_trigram_words_with_int(std::string("tests/trigram_words_int_" + std::to_string(n)), n, reload);
      }

/*      parray<std::pair<char*, int>*> f = pasl::pctl::io::load_seq_from_txt<std::pair<char*, int>*>(std::string("tests/trigram_words_int_txt_10000000"), path_to_data + std::string("trigramSeq_10M_pair_int"), 10000000, true);
      parray<std::pair<char*, int>*> s = pasl::pctl::io::load_seq_from_txt<std::pair<char*, int>*>(std::string("tests/trigram_words_int_txt_10000000"), path_to_data + std::string("trigramSeq_10M_pair_int"), 10000000, false);

      if (s.size() != f.size()) {
        std::cerr << "Not equal\n";
        return;
      }
      for (int i = 0; i < s.size(); i++) {
        if (std::strcmp(f[i]->first, s[i]->first) != 0 || f[i]->second != s[i]->second) {
          std::cerr << "Not equal " << i << "\n";
          std::cerr << f[i]->first << " " << s[i]->first << "\n";
          std::cerr << f[i]->second << " " << s[i]->second << "\n";
          return;
        } 
      }*/
 
      pbbs_pctl_call(measured, a);
      pasl::pctl::parallel_for(0, n, [&] (int i) {
        delete [] a[i]->first;
        delete a[i];
      });
    }
  });
  return 0;
}

/***********************************************************************/
