#include <iostream>
#include <fstream>
#include "ploop.hpp"
#include "parray.hpp"
#include "prandgen.hpp"

#ifndef _PBBS_PCTL_TRIGRAM_GENERATOR_H_
#define _PBBS_PCTL_TRIGRAM_GENERATOR_H_

namespace pasl {
namespace pctl {

#define TRIGRAM_FILE "trigrams.txt"

struct ngram_table {
  int len;

  struct table_entry {
    char str[10];
    int len;
    char chars[27];
    float probs[27];
  };

  table_entry s[27][27];

  int index(char c) {
    if (c == '_') {
      return 26;
    } else {
      return (c - 'a');
    }
  }

  ngram_table() {
    std::ifstream ifile(TRIGRAM_FILE);
    if (!ifile.is_open()) {
      std::cout << "ngram_table: Unable to open trigram file" << std::endl;
      abort();
    } else {
      int i = 0;
      while (!ifile.eof()) {
	table_entry x;
	ifile >> x.str >> x.len;
	float prob_sum = 0.0;
	for (int j = 0; j < x.len; j++) {
	  float prob;
	  ifile >> x.chars[j] >> prob;
	  prob_sum += prob;
	  if (j == x.len - 1) {
            x.probs[j] = 1.0;
          } else {
            x.probs[j] = prob_sum;
          }
	}
	int i0 = index(x.str[0]);
	int i1 = index(x.str[1]);
	if (i0 > 26 || i1 > 26) abort();
	s[i0][i1] = x;
	i++;
      }
      len = i;
    }
  }

  char next(char c0, char c1, int i) {
    int j = 0;
    table_entry e = s[index(c0)][index(c1)];
    double x = pasl::pctl::prandgen::hash<double>(i);
    while (x > e.probs[j]) {
      j++;
    }
//    std::cerr << x << " " << c0 << " " << c1 << " " << e.chars[j] << "\n";
    return e.chars[j];
  }

  int word(int i, char* a, int offset, int max_len) {
    a[offset] = next('_', '_', i);
    if (max_len == 1) {
      return 1;
    }
    a[offset + 1] = next('_', a[offset], i + 1);
    int j = 2;
    while (j < max_len) {
      a[offset + j] = next(a[offset + j - 2], a[offset + j - 1], i + j);
      if (a[offset + j] == '_') {
        break;
      }
      j++;
    }
    return j;
  }

  int word_length(int i, int max_len) {
    char a0 = next('_', '_', i);
    char a1 = next('_', a0, i + 1);
    int j = 2;
    while (a1 != '_' && j < max_len) {
      char tmp = next(a0, a1, i + j);
      a0 = a1;
      a1 = tmp;
      j++;
    }
    return j - 1;
  }

  char* word(int i) {
    int MAX_LEN = 100;
    int len = word_length(i, MAX_LEN);
    char* a = new char[len + 1];
    a[len] = 0;
    int l = word(i, a, 0, len);
    return a;
  }

  std::string string(int s, int e) {
    int n = e - s;
    std::string result;
    result.resize(n);
    int j = 0;
    while (j < n) {
      int l = word(j + s, &result[0], j, n - j);
      result[j + l] = ' ';
      j += l + 1;
    }
    return result;
  }
};

std::string trigram_string(int s, int e) { 
  ngram_table t = ngram_table();
  return t.string(s, e);
}

/*void gen_trigram_string(std::string fname, int n) {
  std::string s = trigram_string(0, n);
  std::ofstream out(fname, std::ofstream::binary);
  io::write_to_file(out, s);
  out.close();
}*/

parray<char*> trigram_words(int s, int e) { 
  int n = e - s;
  parray<char*> a(n);
  ngram_table t = ngram_table();
  pasl::pctl::parallel_for(0, n, [&] (int i) {
    a[i] = t.word(100 * (i + s));
  });
  return a;
}

/*void gen_trigram_words(std::string fname, int n) {
  parray<char*> words = trigram_words(0, n);
  std::ofstream out(fname, std::ofstream::binary);
  io::write_to_file(out, words);
  out.close();
}*/

} //end namespace
} //end namespace
#endif /*! _PBBS_PCTL_TRIGRAM_GENERATOR_H_ !*/