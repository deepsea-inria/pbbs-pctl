/*!
 * \file suffixarray_bench.cpp
 * \brief Benchmarking script for parallel suffix array
 * \date 2016
 * \copyright COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and
 * Michael Rainey. All rights reserved.
 * \license This project is released under the GNU Public License.
 *
 */

#include <math.h>
#include "bench.hpp"
#include "suffixarray.hpp"
#include "loaders.hpp"
#include "pks.h"

/***********************************************************************/

using namespace pasl::pctl;

void pbbs_pctl_call(pbbs::measured_type measured, std::string x) {
  std::string lib_type = deepsea::cmdline::parse_or_default_string("lib_type", "pctl");
  if (lib_type == "pbbs") {
    measured([&] {
      pbbs::suffixArray(&x[0], (int)x.length());
    });
  } else {
    measured([&] {
      pasl::pctl::suffix_array(&x[0], x.length());
    });
  }
}

int main(int argc, char** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    std::string infile = deepsea::cmdline::parse_or_default_string("infile", "");
    if (infile != "") {
      std::string x = pasl::pctl::io::load<std::string>(infile);
      pbbs_pctl_call(measured, x);
      return;
    }
    int test = deepsea::cmdline::parse_or_default_int("test", 0);
    int n = deepsea::cmdline::parse_or_default_int("n", 10000000);
    bool files = deepsea::cmdline::parse_or_default_int("files", 1) == 1;
    bool reload = deepsea::cmdline::parse_or_default_int("reload", 0) == 1;
    std::string path_to_data = deepsea::cmdline::parse_or_default_string("path_to_data", "/home/aksenov/pbbs/sequenceData/data/");
    system("mkdir tests");
    if (test == 0) {
      std::string a;
      if (files) {
        a = pasl::pctl::io::load_string_from_txt("tests/trigram_string_txt_10000000", path_to_data + "trigramString_10000000", reload);
      } else {
        a = pasl::pctl::io::load_trigram_string("tests/trigram_string_" + std::to_string(n), n, reload);
      }
      pbbs_pctl_call(measured, a);
    } else if (test == 1) {
      std::string a;
      a = pasl::pctl::io::load_string_from_txt("tests/chr22.dna.bin", path_to_data + "chr22.dna", reload);
      pbbs_pctl_call(measured, a);
    } else if (test == 2) {
      std::string a;
      a = pasl::pctl::io::load_string_from_txt("tests/etext99.bin", path_to_data + "etext99", reload);
      pbbs_pctl_call(measured, a);
    } else if (test == 3) {
      std::string a;
      a = pasl::pctl::io::load_string_from_txt("tests/wikisamp.xml.bin", path_to_data + "wikisamp.xml", reload);
      pbbs_pctl_call(measured, a);
    }
  });
  return 0;
}

/***********************************************************************/
