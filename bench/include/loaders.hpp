#include <string>
#include "trigram_generator.hpp"
#include "rays_generator.hpp"
#include "serialization.hpp"
#include "sequencedata.hpp"
#include "geometrydata.hpp"
#include "kdtree.hpp"

#ifndef _PCTL_PBBS_LOADERS_H_
#define _PCTL_PBBS_LOADERS_H_

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
    return point2d(std::stod(words[p++]), std::stod(words[p++]));
  }
};

template <>
struct load_item<point3d> {
  point3d operator()(std::vector<std::string>& words, int& p) {
    return point3d(std::stod(words[p++]), std::stod(words[p++]), std::stod(words[p++]));
  }
};

template <class Item1, class Item2>
struct load_item<std::pair<Item1, Item2>> {
  std::pair<Item1, Item2> operator()(std::vector<std::string>& words, int& p) {
    return make_pair(load_item<Item1>()(words, p), load_item<Item2>()(words, p));
  }
};

template <class Item1, class Item2>
struct load_item<std::pair<Item1, Item2>*> {
  std::pair<Item1, Item2>* operator()(std::vector<std::string>& words, int& p) {
    return new std::pair<Item1, Item2>(load_item<Item1>()(words, p), load_item<Item2>()(words, p));
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
  }

  parray<Item> operator()(std::vector<std::string>& words, int& p, int n) {
//    std::cerr << words.size() << " " << p << " " << n << std::endl; 
    parray<Item> result(n);
    for (int i = 0; i < n; i++) {
      result[i] = load_item<Item>()(words, p);
    }
    std::cerr << "Finished loading\n";
    return result;
  }
};

template <class Item>
Item read_from_txt(std::string file) {
  std::vector<std::string> words = string_to_words(read_string_from_txt(file));
  int p = 1;
  return load_item<Item>()(words, p);
}

template <class Item>
parray<Item> read_seq_from_txt(std::string file, int n) {
  std::cerr << "Loading from file\n";
  std::vector<std::string> words = string_to_words(read_string_from_txt(file));
  int p = 1;
  parray<Item> result = load_item<parray<Item>>()(words, p, n);
  std::cerr << "Sequence loaded: " << result.size() << "\n";
  return result;
}

template <class Generator_fct, class Item = typename std::result_of<Generator_fct&()>::type>
Item load(std::string file, const Generator_fct& gen, bool regenerate = false) {
  std::ifstream in(file, std::ifstream::binary);
  if (!regenerate && in.good()) {
    return read_from_file<Item>(in);
  } else {
    Item result = gen();
    std::ofstream out(file, std::ofstream::binary);
    std::cerr << "Try to write into file\n";
    write_to_file(out, result);
    out.close();
    return result;
  }
}

template <class Item>
parray<Item> load_seq_from_txt(std::string file, std::string seq_file, int n, bool regenerate = false) {
  return load(file, [&] { return read_seq_from_txt<Item>(seq_file, n); }, regenerate);
}

template <class Item>
parray<Item> load_random_seq(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::sequencedata::rand<Item>(0, n); }, regenerate);
}

parray<int> load_random_bounded_seq(std::string file, int n, int m, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::sequencedata::rand_int_range<int>(0, n, m); }, regenerate);
}

parray<int> load_random_bounded_seq(std::string file, int n, bool regenerate = false) {
  return load_random_bounded_seq(file, n, n, regenerate);
}

parray<std::pair<int, int>> load_random_bounded_seq_with_int(std::string file, int n, int r1, int r2, bool regenerate = false) {
  return load(file, [&] {
    parray<int> f = pasl::pctl::sequencedata::rand_int_range<int>(0, n, r1);
    return parray<std::pair<int, int>>(n, [&] (int i) {
      int v = pasl::pctl::prandgen::hash<int>(n + i) % r2;
      return make_pair(f[i], v);
    });
  }, regenerate);
}

parray<std::pair<int, int>> load_random_bounded_seq_with_int(std::string file, int n, int r, bool regenerate = false) {
  return load_random_bounded_seq_with_int(file, n, n, r, regenerate);
}

template <class Item>
parray<Item> load_random_almost_sorted_seq(std::string file, int n, int nb_swaps, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::sequencedata::almost_sorted<Item>(0, n, nb_swaps); }, regenerate);
}

template <class Item>
parray<Item> load_random_exp_dist_seq(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::sequencedata::exp_dist<Item>(0, n); }, regenerate);
}

parray<char*> load_trigram_words(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::trigram_words(0, n); }, regenerate);
}

parray<std::pair<char*, int>*> load_trigram_words_with_int(std::string file, int n, int range, bool regenerate = false) {
  return load(file, [&] {
    parray<char*> f = pasl::pctl::trigram_words(0, n);
    return parray<std::pair<char*, int>*>(n, [&] (int i) {
      int v = pasl::pctl::prandgen::hash<int>(n + i) % range;
      return new std::pair<char*, int>(f[i], v);
    });
  }, regenerate);
}

parray<std::pair<char*, int>*> load_trigram_words_with_int(std::string file, int n, bool regenerate = false) {
  return load_trigram_words_with_int(file, n, n, regenerate);
}

parray<point2d> load_points_uniform_2d(std::string file, int n, bool inSphere, bool onSphere, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::uniform2d(inSphere, onSphere, n); }, regenerate);
}

parray<point3d> load_points_uniform_3d(std::string file, int n, bool inSphere, bool onSphere, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::uniform3d<int, unsigned int>(inSphere, onSphere, n); }, regenerate);
}

parray<point2d> load_points_plummer_2d(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::plummer2d(n); }, regenerate);
}

parray<point3d> load_points_plummer_3d(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::plummer3d<int, unsigned int>(n); }, regenerate);
}

std::string load_trigram_string(std::string file, int n, bool regenerate = false) {
  return load(file, [&] { return pasl::pctl::trigram_string(0, n); }, regenerate);
}

std::string load_string_from_txt(std::string file, std::string txt_file, bool regenerate = false) {
  return load(file, [&] { return read_string_from_txt(txt_file); }, regenerate);
}

std::pair<parray<point3d>, parray<triangle>> load_triangles_from_txt(std::string file) {
  std::string s = read_string_from_txt(file);
  std::vector<std::string> words = string_to_words(s);
  int p = 1;
  parray<point3d> points(std::stoi(words[p++]));
  parray<triangle> triangles(std::stoi(words[p++]));
  for (int i = 0; i < points.size(); i++) {
    new (&points[i]) point3d(std::stod(words[p++]), std::stod(words[p++]), std::stod(words[p++]));
  }
  for (int i = 0; i < triangles.size(); i++) {
    new (&triangles[i]) triangle(std::stoi(words[i++]) - 1, std::stoi(words[p++]) - 1, std::stoi(words[p++]) - 1);
  }
  return make_pair(points, triangles);
}

ray_cast_test load_ray_cast_test(std::string file, std::string triangles_file, bool regenerate = false) {
  std::ifstream in(file, std::ifstream::binary);
  ray_cast_test test;
  if (!regenerate && in.good()) {
    test.points = read_from_file<parray<point3d>>(in);
    test.triangles = read_from_file<parray<triangle>>(in);
    test.rays = read_from_file<parray<ray<point3d>>>(in);
  } else {
    auto p = load_triangles_from_txt(triangles_file);
    test.points = p.first;
    test.triangles = p.second;
    test.rays = generate_rays(test.triangles.size(), test.points);
    
    std::ofstream out(file, std::ofstream::binary);
    write_to_file(out, test.points);
    write_to_file(out, test.triangles);
    write_to_file(out, test.rays);
  }
}

} //end namespace
} //end namespace
} //end namespace

#endif /*! _PCTL_PBBS_LOADERS_H_ */