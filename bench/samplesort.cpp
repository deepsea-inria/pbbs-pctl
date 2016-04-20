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
#include "samplesort.hpp"
#include "loaders.hpp"

/***********************************************************************/

/*---------------------------------------------------------------------*/

template <class Item>
using parray = pasl::pctl::parray<Item>;

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    int test = pasl::util::cmdline::parse_or_default_int("test", 0);
    int n = pasl::util::cmdline::parse_or_default_int("n", 10000000);
    bool files = pasl::util::cmdline::parse_or_default_int("files", 1) == 1;
    std::string path_to_data = pasl::util::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
    system("mkdir tests");
    if (test == 0) {
      parray<double> a;
      if (!files) {
        a = pasl::pctl::io::load_random_seq<double>(std::string("tests/random_seq_") + std::to_string(n), n);
      } else {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_seq_txt_10000000"), path_to_data + std::string("randomSeq_10M_double"), 10000000);
      }
      measured([&] {
        pasl::pctl::sample_sort(a.begin(), (int)a.size(), std::less<double>());
      });
    } else if (test == 1) {
      parray<double> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_exp_dist_seq_txt_10000000"), path_to_data + std::string("exptSeq_10M_double"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_exp_dist_seq<double>(std::string("tests/random_exp_dist_seq_") + std::to_string(n), n);
      }
      measured([&] {
        pasl::pctl::sample_sort(a.begin(), (int)a.size(), std::less<double>());
      });
    } else if (test == 2) {
      parray<double> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<double>(std::string("tests/random_almost_sorted_seq_txt_10000000"), path_to_data + std::string("almostSortedSeq_10M_double"), 10000000);
      } else {
        a = pasl::pctl::io::load_random_almost_sorted_seq<double>(std::string("tests/random_almost_sorted_seq_") + std::to_string(n), n, (int)sqrt(n));
      }
      measured([&] {
        pasl::pctl::sample_sort(a.begin(), (int)a.size(), std::less<double>());
      });
    } else if (test == 3) {
      parray<char*> a;
      if (files) {
        a = pasl::pctl::io::load_seq_from_txt<char*>(std::string("tests/trigram_words_txt_10000000"), path_to_data + std::string("trigramSeq_10M"), 10000000);
      } else {
        a = pasl::pctl::io::load_trigram_words(std::string("tests/trigram_words_") + std::to_string(n), n);
      }
      measured([&] {
        pasl::pctl::sample_sort(a.begin(), (int)a.size(), [&] (char* a, char* b) {
          return std::strcmp(a, b) < 0;
        });
      });
      pasl::pctl::parallel_for(0, n, [&] (int i) {
        delete [] a[i];
      });
    }
  });
  return 0;
}

/***********************************************************************/
