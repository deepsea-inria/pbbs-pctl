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

template <class Generator_fct, class Item = typename std::result_of<Generator_fct&()>::type>
Item load(std::string file, const Generator_fct& gen, bool regenerate = false) {
  std::ifstream in(file, std::ifstream::binary);
  if (!regenerate && in.good()) {
    return read_from_file<Item>(in);
  } else {
    Item result = gen();
    std::ofstream out(file, std::ofstream::binary);
    write_to_file(out, result);
    out.close();
    return result;
  }
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

parray<std::pair<char*, int>> load_trigram_words_with_int(std::string file, int n, int range, bool regenerate = false) {
  return load(file, [&] {
    parray<char*> f = pasl::pctl::trigram_words(0, n);
    return parray<std::pair<char*, int>>(n, [&] (int i) {
      int v = pasl::pctl::prandgen::hash<int>(n + i) % range;
      return make_pair(f[i], v);
    });
  }, regenerate);
}

parray<std::pair<char*, int>> load_trigram_words_with_int(std::string file, int n, bool regenerate = false) {
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
  while ((pos == s.find_first_of(" \t\n\r", prev)) != std::string::npos) {
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