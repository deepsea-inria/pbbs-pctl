/*!
 * \file blockradixsort_bench.cpp
 * \brief Benchmarking script for parallel sorting algorithms
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
#include "blockradixsort.hpp"
#include "loaders.hpp"
#include "blockRadixSort.h"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

template <class Item>
void pbbs_pctl_call(pbbs::measured_type measured, parray<Item>& x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    measured([&] {
      pbbs::integerSort<int>(&x[0], (int)x.size());
    });
  } else {
    measured([&] {
      pasl::pctl::integer_sort(x.begin(), (int)x.size());
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
      d.add("array_pair_int_int", [&]  {
        parray<std::pair<int, int>> x = pasl::pctl::io::load<parray<std::pair<int, int>>>(infile);
        pbbs_pctl_call(measured, x);
      });
      d.dispatch("type");
      return;
    }

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
      pbbs_pctl_call(measured, a);
    } else if (test == 1) {
      parray<int> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<int>(std::string("tests/random_exp_dist_seq_int_txt_10000000"), path_to_data + std::string("exptSeq_10M_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<int>(std::string("tests/random_exp_dist_seq_int_") + std::to_string(n), n, reload);
      }
      pbbs_pctl_call(measured, a);
    } else if (test == 2) {
      parray<std::pair<int, int>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<std::pair<int, int>>(std::string("tests/random_seq_int_int_txt_10000000"), path_to_data + std::string("randomSeq_10M_int_pair_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq_with_int(std::string("tests/random_seq_int_int_") + std::to_string(n), n, n, n, reload);
      }
      pbbs_pctl_call(measured, a);
    } else if (test == 3) {
      parray<std::pair<int, int>> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<std::pair<int, int>>(std::string("tests/random_seq_int_int_txt_10000000_256"), path_to_data + std::string("randomSeq_10M_256_int_pair_int"), 10000000, reload);
      } else {
        a = pasl::pctl::io::load_random_bounded_seq_with_int(std::string("tests/random_seq_int_int_") + std::to_string(n) + "_256", n, 256, n, reload);
      }
      pbbs_pctl_call(measured, a);
    }
  });
  return 0;
}

/***********************************************************************/
