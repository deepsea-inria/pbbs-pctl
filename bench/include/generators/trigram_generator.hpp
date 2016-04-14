#include "serialization.hpp"

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
    ifstream ifile(TRIGRAM_FILE);
    if (!ifile.is_open()) {
      std::cout << "ngram_table: Unable to open trigram file" << endl;
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
    return e.chars[j];
  }

  int word(int i, std::string a, int offset, int max_len) {
    a[offset] = next('_', '_', i);
    a[offset + 1] = next('_', a[offset], i + 1);
    int j = 2;
    while (j < max_len && offset + j < a.length() && a[offset + j] != '_') {
      a[offset + j] = next(a[offset + j - 2], a[offset + j - 1], i + j);
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
    return j;
  }

  std::string word(int i) {
    int MAX_LEN = 100;
    std::string a;
    a.resize(MAX_LEN);
    int l = word(i, a, 0, MAX_LEN);
    a.resize(l);
    return a;
  }

  std::string string(int s, int e) {
    int n = e - s;
    std::string result;
    result.resize(n);
    int j = 0;
    while (j < n) {
      int l = word(j + s, result, j, n - j);
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

void gen_trigram_string(std::string fname, int n) {
  std::string s = trigram_string(0, n);
  std::ofstream out(fname, std::ofstream::binary);
  io::write_to_file(out, s);
  out.close();
}

parray<std::string> trigram_words(int s, int e) { 
  int n = e - s;
  parray<std::string> a(n);
  ngram_table t = ngram_table();
  pasl::pctl::parallel_for(0, n, [&] (int i) {
    a[i] = t.word(100 * (i + s));
  });
  return a;
}

void gen_trigram_words(std::string fname, int n) {
  parray<std::string> words = trigram_words(0, n);
  std::ofstream out(fname, std::ofstream::binary);
  io::write_to_file(out, words);
  out.close();
}

} //end namespace
} //end namespace
#endif /*! _PBBS_PCTL_TRIGRAM_GENERATOR_H_ !*/