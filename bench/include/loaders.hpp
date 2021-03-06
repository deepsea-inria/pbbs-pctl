#include <string>
#include "trigram_generator.hpp"
#include "rays_generator.hpp"
#include "serializationtxt.hpp"
#include "serializationbin.hpp"
#include "sequencedata.hpp"
#include "geometrydata.hpp"
#include "geometry.hpp"
#include "cmdline.hpp"

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
parray<Item> load_seq_from_txt(std::string file, std::string seq_file, int n, bool regenerate = false) {
  return load(file, [&] { return read_from_txt_file<parray<Item>>(seq_file); }, regenerate);
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
  return load(file, [&] { return read_from_txt_file<std::string>(txt_file); }, regenerate);
}

ray_cast_test load_ray_cast_test(std::string file, std::string triangles_file, std::string rays_file, bool regenerate = false) {
  std::ifstream in(file, std::ifstream::binary);
  ray_cast_test test;
  if (!regenerate && in.good()) {
    test.points = read_from_file<parray<point3d>>(in);
    test.triangles = read_from_file<parray<triangle>>(in);
    test.rays = read_from_file<parray<ray<point3d>>>(in);
  } else {
    test = read_from_txt_files<ray_cast_test>(triangles_file, rays_file);
    
    std::ofstream out(file, std::ofstream::binary);
    write_to_file(out, test.points);
    write_to_file(out, test.triangles);
    write_to_file(out, test.rays);
    return test;
  }
  return test;
}

void parse_filename(std::string fname, std::string& base, std::string& extension) {
  assert(fname != "");
  int pos = fname.find_last_of(".");
  base = fname.substr(0, pos);
  extension = fname.substr(pos + 1);
}

template <class Item>
struct load_struct {
  Item operator()(std::string infile, std::string base, std::string extension) {
    if (extension == "txt") {
      return io::read_from_txt_file<Item>(infile);
    } else if (extension == "bin") {
      return io::read_from_file<Item>(infile);
    } else {
      assert(false);
    }
  }
};

template <class intT>
struct load_struct<graph::graph<intT>> {
  graph::graph<intT> operator()(std::string infile, std::string base, std::string extension) {
    if (extension == "txt") {
      return io::read_from_txt_file<graph::graph<intT>>(infile);
    } else if (extension == "bin") {
      return io::read_from_file<graph::graph<intT>>(infile);
    } else if (extension == "adj_bin") {
      return io::read_from_pasl_file<graph::graph<intT>>(infile);
    } else {
      assert(false);
    }
  }
};

template <class Item>
Item load(std::string infile) {
  std::string base;
  std::string extension;
  io::parse_filename(infile, base, extension);
  return load_struct<Item>()(infile, base, extension);
}

template <class Item>
Item load(std::string infile, std::string infile2) {
  std::string base;
  std::string extension;
  io::parse_filename(infile, base, extension);
  if (extension == "txt") {
    return io::read_from_txt_files<Item>(infile, infile2);
  } else {
    assert(false);
  }
}

template <class Item> 
Item load_from_file(std::string key1, std::string key2 = "") {
  std::string infile = deepsea::cmdline::parse_string(key1);
  std::string infile2 = deepsea::cmdline::parse_or_default<std::string>(key2, "");
  if (infile2 != "") {
    return load<Item>(infile, infile2);
  } else {
    return load<Item>(infile);
  }
}

} //end namespace
} //end namespace
} //end namespace

#endif /*! _PCTL_PBBS_LOADERS_H_ */
