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

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = pasl::util::cmdline::parse_or_default_int("test", 0);
    int n = pasl::util::cmdline::parse_or_default_int("n", 10000000);
    int m = pasl::util::cmdline::parse_or_default_int("m", 1000000);
    bool files = pasl::util::cmdline::parse_or_default_int("files", 1) == 1;
    system("mkdir tests");
    if (test == 0) {
      parray<int> a;
      if (!files) {
        a = pasl::pctl::io::load_random_seq<int>(std::string("tests/random_seq_int_") + std::to_string(n), n);
      } else {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_seq_int_txt_10000000"), std::string("tests/randomSeq_10M_int"), 10000000);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 1) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_bounded_seq_txt_10000000_100000"), std::string("tests/randomSeq_10M_100K_int"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq(std::string("tests/random_bounded_seq_") + std::to_string(n) + "_" + std::to_string(m), n, m);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 2) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_exp_dist_seq_int_txt_10000000"), std::string("tests/exptSeq_10M_int"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<int>(std::string("tests/random_exp_dist_seq_int_") + std::to_string(n), n);
      }
      measured([&] {
        pasl::pctl::remove_duplicates(a);
      });
    } else if (test == 3) {
      parray<char*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<char*>(std::string("tests/trigram_words_txt_10000000"), std::string("tests/trigramSeq_10M"), 10000000);
      } else {
        a = pasl::pctl::io::load_trigram_words(std::string("tests/trigram_words_") + std::to_string(n), n);
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
        a = pasl::pctl::io::load_seq_from_txt<std::pair<char*, int>*>(std::string("tests/trigram_words_int_txt_10000000"), std::string("tests/trigramSeq_10M_pair_int"), 10000000);
      } else {
        a = pasl::pctl::io::load_trigram_words_with_int(std::string("test/trigram_words_int_txt_" + std::to_string(n)), n);
      }
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
