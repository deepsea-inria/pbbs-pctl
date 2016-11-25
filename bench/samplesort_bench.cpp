/*!
 * \file samplesort_bench.cpp
 * \brief Benchmarking script for parallel sample sort
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
#include "samplesort.hpp"
#include "loaders.hpp"
#include "sampleSort.h"
#undef parallel_for // later: understand how this macro is leaking into this module...
/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

template <class Item, class Compare_fct>
void pbbs_pctl_call(pbbs::measured_type measured, parray<Item>& x, const Compare_fct& compare) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    measured([&] {
      pbbs::sampleSort(x.begin(), (int)x.size(), compare);
    });
  } else {
    measured([&] {
      pasl::pctl::sample_sort(x.begin(), (int)x.size(), compare);
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default<std::string>("infile", "");
    if (infile != "") {
      deepsea::cmdline::dispatcher d;
      d.add("array_double", [&] {
        parray<double> x = pasl::pctl::io::load<parray<double>>(infile);
        pbbs_pctl_call(measured, x, std::less<double>());
      });
      d.add("array_int", [&] {
        parray<int> x = pasl::pctl::io::load<parray<int>>(infile);
        pbbs_pctl_call(measured, x, std::less<int>());
      });
      d.add("array_string", [&] {
        parray<char*> x = pasl::pctl::io::load<parray<char*>>(infile);
        pbbs_pctl_call(measured, x, [&] (char* a, char* b) { return std::strcmp(a, b) < 0; });
        for (int i = 0; i < x.size(); i++) {
          delete [] x[i];
        }
      });
      d.dispatch("type");
      return;
    }

    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
    system("mkdir tests");

    if (test == 0) {
      parray<double> a;
      if (!files) {
        a = pasl::pctl::io::load_random_seq<double>(std::string("tests/random_seq_") + std::to_string(n), n);
      } else {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_seq_txt_10000000"), path_to_data + std::string("randomSeq_10M_double"), 10000000);
      }
      pbbs_pctl_call(measured, a, std::less<double>());
      for (int i = 1; i < a.size(); i++) {
        if (a[i] < a[i - 1]) {
          std::cerr << "ACHTUNG!\n";
          return;
        }
      }
    } else if (test == 1) {
      parray<double> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_exp_dist_seq_txt_10000000"), path_to_data + std::string("exptSeq_10M_double"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<double>(std::string("tests/random_exp_dist_seq_") + std::to_string(n), n);
      }
      pbbs_pctl_call(measured, a, std::less<double>());
    } else if (test == 2) {
      parray<double> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_almost_sorted_seq_txt_10000000"), path_to_data + std::string("almostSortedSeq_10M_double"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_almost_sorted_seq<double>(std::string("tests/random_almost_sorted_seq_") + std::to_string(n), n, (int)sqrt(n));
      }
      pbbs_pctl_call(measured, a, std::less<double>());
    } else if (test == 3) {
      parray<char*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<char*>(std::string("tests/trigram_words_txt_10000000"), path_to_data + std::string("trigramSeq_10M"), 10000000);
      } else {
        a = pasl::pctl::io::load_trigram_words(std::string("tests/trigram_words_") + std::to_string(n), n);
      }
      pbbs_pctl_call(measured, a, [&] (char* a, char* b) {
            return std::strcmp(a, b) < 0;
      });
      pasl::pctl::parallel_for(0, n, [&] (int i) {
        delete [] a[i];
      });
    }
  });
  return 0;
}

/***********************************************************************/
