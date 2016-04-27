
#include <string>
#include <sstream>
#include <fstream>

#include "geometry.hpp"

#ifndef _PCTL_IO_SHARED_H_
#define _PCTL_IO_SHARED_H_

namespace pasl {
namespace pctl {
namespace io {
  
std::string read_string_from_txt(std::string file) {
  ifstream in(file);
  in.seekg(0, std::ios::end);
  size_t size = in.tellg();
  std::string buffer;
  buffer.resize(size);
  in.seekg(0);
  in.read(&buffer[0], size);
  return buffer;
}

std::vector<std::string> string_to_words(std::string s) {
  std::vector<std::string> words;
  size_t pos = 0;
  size_t prev = 0;
  while ((pos = s.find_first_of(" \t\n\r", prev)) != std::string::npos) {
    if (pos > prev) {
      words.push_back(s.substr(prev, pos - prev));
    }
    prev = pos + 1;
  }
  if (prev < s.length()) {
    words.push_back(s.substr(prev, std::string::npos));
  }
  return words;
}

template <class Item>
struct load_item {};

template <>
struct load_item<int> {
  int operator()(std::vector<std::string>& words, int& p) {
    return std::stoi(words[p++]);
  }
};

template <>
struct load_item<double> {
  double operator()(std::vector<std::string>& words, int& p) {
    return std::stod(words[p++]);
  }
};

template <>
struct load_item<char*> {
  char* operator()(std::vector<std::string>& words, int& p) {
    char* result = new char[words[p].length() + 1];
    result[words[p].length()] = '\0';
    std::copy(words[p].begin(), words[p].end(), result);
    p++;
    return result;
  }
};

template <>
struct load_item<std::string> {
  std::string operator()(std::vector<std::string>& words, int& p) {
    return words[p++];
  }
};
  
template <>
struct load_item<point2d> {
  point2d operator()(std::vector<std::string>& words, int& p) {
    double x = std::stod(words[p++]);
    double y = std::stod(words[p++]);
    return point2d(x, y);
  }
};

template <>
struct load_item<point3d> {
  point3d operator()(std::vector<std::string>& words, int& p) {
    double x = std::stod(words[p++]);
    double y = std::stod(words[p++]);
    double z = std::stod(words[p++]);
    return point3d(x, y, z);
  }
};

template <class Item1, class Item2>
struct load_item<std::pair<Item1, Item2>> {
  std::pair<Item1, Item2> operator()(std::vector<std::string>& words, int& p) {
    Item1 i1 = load_item<Item1>()(words, p);
    Item2 i2 = load_item<Item2>()(words, p);
    return make_pair(i1, i2);
  }
};

template <class Item1, class Item2>
struct load_item<std::pair<Item1, Item2>*> {
  std::pair<Item1, Item2>* operator()(std::vector<std::string>& words, int& p) {
    Item1 i1 = load_item<Item1>()(words, p);
    Item2 i2 = load_item<Item2>()(words, p);
    return new std::pair<Item1, Item2>(i1, i2);
  }
};

template <class Item>
struct load_item<parray<Item>> {
  parray<Item> operator()(std::vector<std::string>& words, int& p) {
    int size = std::stoi(words[p++]);
    parray<Item> result(size);
    for (int i = 0; i < size; i++) {
      result[i] = load_item<Item>()(words, p);
    }
    return result;
  }
  
  parray<Item> operator()(std::vector<std::string>& words, int& p, int n) {
    parray<Item> result(n);
    for (int i = 0; i < n; i++) {
      result[i] = load_item<Item>()(words, p);
    }
    std::cerr << "Finished loading\n";
    return result;
  }
};

template <class Item>
parray<Item> read_seq_from_txt(std::string file) {
  std::vector<std::string> words = string_to_words(read_string_from_txt(file));
  int p = 1;
  return load_item<Item>()(words, p);
}
  
void parse_filename(std::string fname, std::string& base, std::string& extension) {
  assert(fname != "");
  std::stringstream ss(fname);
  std::getline(ss, base, '.');
  std::getline(ss, extension);
}
  
} // end namespace
} // end namespace
} // end namespace

#endif