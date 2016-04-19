/*!
 * \file psort.cpp
 * \brief Benchmarking script for parallel sorting algorithms
 * \date 2015
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

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

//std::string path_to_data = "/home/aksenov/pbbs/sequenceData/data/";

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = pasl::util::cmdline::parse_or_default_int("test", 0);
    int n = pasl::util::cmdline::parse_or_default_int("n", 10000000);
    int m = pasl::util::cmdline::parse_or_default_int("m", 1000000);
    bool files = pasl::util::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = pasl::util::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = pasl::util::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
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

      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 1) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_bounded_seq_txt_10000000_100000"), path_to_data + std::string("randomSeq_10M_100K_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq(std::string("tests/random_bounded_seq_") + std::to_string(n) + "_" + std::to_string(m), n, m, reload);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 2) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_exp_dist_seq_int_txt_10000000"), path_to_data + std::string("exptSeq_10M_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<int>(std::string("tests/random_exp_dist_seq_int_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 3) {
      parray<char*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<char*>(std::string("tests/trigram_words_txt_10000000"), path_to_data + std::string("trigramSeq_10M"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_trigram_words(std::string("tests/trigram_words_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
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

      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
      pasl::pctl::parallel_for(0, n, [&] (int i) {
        delete [] a[i]->first;
        delete a[i];
      });
    }
  });
  return 0;
}

/***********************************************************************/
