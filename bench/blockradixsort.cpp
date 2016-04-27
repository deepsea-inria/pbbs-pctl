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
#include "blockradixsort.hpp"
#include "loaders.hpp"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = deepsea::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
    system("mkdir tests");
    if (test == 0) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_seq_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_seq<int>(std::string("tests/random_seq_int_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::integer_sort(a.begin(), (int)a.size());
      });
    } else if (test == 1) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_exp_dist_seq_int_txt_10000000"), path_to_data + std::string("exptSeq_10M_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<int>(std::string("tests/random_exp_dist_seq_int_") + std::to_string(n), n, reload);
      }
      measured([&] {
        pasl::pctl::integer_sort(a.begin(), (int)a.size());
      });
    } else if (test == 2) {
      parray<std::pair<int, int>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<std::pair<int, int>>(std::string("tests/random_seq_int_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int_pair_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq_with_int(std::string("tests/random_seq_int_int_") + std::to_string(n), n, n, n, reload);
      }
      measured([&] {
        pasl::pctl::integer_sort(a.begin(), (int)a.size());
      });
    } else if (test == 3) {
      parray<std::pair<int, int>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<std::pair<int, int>>(std::string("tests/random_seq_int_int_txt_10000000_256"), path_to_data + std::string("randomSeq_10M_256_int_pair_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq_with_int(std::string("tests/random_seq_int_int_") + std::to_string(n) + "_256", n, 256, n, reload);
      }
      measured([&] {
        pasl::pctl::integer_sort(a.begin(), (int)a.size());
      });
    }
  });
  return 0;
}

/***********************************************************************/
