
#include <string>
#include <sstream>
#include <fstream>

#include "geometry.hpp"
#include "teststructs.hpp"

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
struct read_from_txt_file_struct {
  Item operator()(std::vector<std::string>& words, int& p) {
    assert(false);
  }
};

template <class Item>
struct read_from_txt_files_struct {
  Item operator()(std::vector<std::string>& fwords, int& f, std::vector<std::string>& swords, int& s) {
    assert(false);
  }
};

template <class Item>
struct write_to_txt_file_struct {
  void operator()(std::ofstream& out, Item& item) {
    out << item;
  }
};

template <class Item>
struct write_to_txt_files_struct {
  void operator()(std::ofstream& first, std::ofstream& second, Item& item) {
    assert(false);
  }
};

template <>
struct read_from_txt_file_struct<int> {
  int operator()(std::vector<std::string>& words, int& p) {
    return std::stoi(words[p++]);
  }
};

template <>
struct read_from_txt_file_struct<double> {
  double operator()(std::vector<std::string>& words, int& p) {
    return std::stod(words[p++]);
  }
};

template <>
struct read_from_txt_file_struct<std::string> {
  std::string operator()(std::vector<std::string>& words, int& p) {
    return words[p++];
  }
};

template <>
struct read_from_txt_file_struct<point2d> {
  point2d operator()(std::vector<std::string>& words, int& p) {
    double x = std::stod(words[p++]);
    double y = std::stod(words[p++]);
    return point2d(x, y);
  }
};

template <>
struct read_from_txt_file_struct<point3d> {
  point3d operator()(std::vector<std::string>& words, int& p) {
    double x = std::stod(words[p++]);
    double y = std::stod(words[p++]);
    double z = std::stod(words[p++]);
    return point3d(x, y, z);
  }
};

template <>
struct read_from_txt_file_struct<triangle> {
  triangle operator()(std::vector<std::string>& words, int& p) {
    int a = std::stoi(words[p++]);
    int b = std::stoi(words[p++]);
    int c = std::stoi(words[p++]);
    return triangle(a - 1, b - 1, c - 1);
  }
};

template <>
struct write_to_txt_file_struct<triangle> {
  void operator()(std::ofstream& out, triangle& x) {
    out << (x.vertices[0] + 1) << " " << (x.vertices[1] + 1) << " " << (x.vertices[2] + 1);
  }
};

template <class Item>
struct read_from_txt_file_struct<Item*> {
  Item* operator()(std::vector<std::string>& words, int& p, long n) {
    Item* result = (Item*)malloc(sizeof(Item) * n);
    for (int i = 0; i < n; i++) {
      result[i] = read_from_txt_file_struct<Item>()(words, p);
    }
    return result;
  }
};

template <class Item>
struct write_to_txt_file_struct<Item*> {
  void operator()(std::ofstream& out, Item* items, int n) {
    for (int i = 0; i < n; i++) {
      write_to_txt_file_struct<Item>()(out, items[i]);
    }
  }

  void operator()(std::ofstream& words, Item* items){
    assert(false);
  }
};

template <>
struct read_from_txt_file_struct<char*> {
  char* operator()(std::vector<std::string>& words, int& p) {
    char* result = new char[words[p].length() + 1];
    result[words[p].length()] = '\0';
    std::copy(words[p].begin(), words[p].end(), result);
    p++;
    return result;
  }
};

template <class Item1, class Item2>
struct read_from_txt_file_struct<std::pair<Item1, Item2>> {
  std::pair<Item1, Item2> operator()(std::vector<std::string>& words, int& p) {
    Item1 i1 = read_from_txt_file_struct<Item1>()(words, p);
    Item2 i2 = read_from_txt_file_struct<Item2>()(words, p);
    return make_pair(i1, i2);
  }
};

template <class Item1, class Item2>
struct write_to_txt_file_struct<std::pair<Item1, Item2>> {
  void operator()(std::ofstream& out, std::pair<Item1, Item2>& x) {
    write_to_txt_file_struct<Item1>()(out, x.first);
    out << " ";
    write_to_txt_file_struct<Item2>()(out, x.second);
  }
};

template <class Item1, class Item2>
struct read_from_txt_file_struct<std::pair<Item1, Item2>*> {
  std::pair<Item1, Item2>* operator()(std::vector<std::string>& words, int& p) {
    Item1 i1 = read_from_txt_file_struct<Item1>()(words, p);
    Item2 i2 = read_from_txt_file_struct<Item2>()(words, p);
    return new std::pair<Item1, Item2>(i1, i2);
  }
};

template <class Item1, class Item2>
struct write_to_txt_file_struct<std::pair<Item1, Item2>*> {
  void operator()(std::ofstream& out, std::pair<Item1, Item2>*& x) {
    write_to_txt_file_struct<std::pair<Item1, Item2>>()(out, *x);
  }
};

template <class Item>
struct read_from_txt_file_struct<parray<Item>> {
  parray<Item> operator()(std::vector<std::string>& words, int& p, int n) {
    parray<Item> result(n);
    for (int i = 0; i < n; i++) {
      result[i] = read_from_txt_file_struct<Item>()(words, p);
    }
    std::cerr << "Finished loading\n";
    return result;
  }

  parray<Item> operator()(std::vector<std::string>& words, int& p) {
    std::vector<Item> result;
    while (p < words.size()) {
      result.push_back(read_from_txt_file_struct<Item>()(words, p));
    }
    return parray<Item>(&result[0], &result[0] + result.size());
  }

};

template <class Item>
struct write_to_txt_file_struct<parray<Item>> {
  void operator()(std::ofstream& out, parray<Item>& x) {
    for (int i = 0; i < x.size(); i++) {
      write_to_txt_file_struct<Item>()(out, x[i]);
      out << std::endl;
    }
  }
};

template <class Point>
struct read_from_txt_file_struct<triangles<Point>> {
  triangles<Point> operator()(std::vector<std::string>& words, int& p) {
    triangles<Point> triangles;
    triangles.num_points = std::stoi(words[p++]);
    triangles.num_triangles = std::stoi(words[p++]);
    triangles.p = read_from_txt_file_struct<Point*>()(words, p, triangles.num_points);
    triangles.t = read_from_txt_file_struct<triangle*>()(words, p, triangles.num_triangles);
    return triangles;
  }
};

template <class Point>
struct write_to_txt_file_struct<triangles<Point>> {
  void operator()(std::ofstream& out, triangles<Point>& t) {
    out << t.num_points << " " << t.num_triangles << std::endl;
    write_to_txt_file_struct<Point*>()(out, t.p, t.num_points);
    write_to_txt_file_struct<triangle*>()(out, t.t, t.num_triangles);
  }
};

template <>
struct read_from_txt_files_struct<ray_cast_test> {
  ray_cast_test operator()(std::vector<std::string>& fwords, int& f, std::vector<std::string>& swords, int& s) {
    int pn = std::stoi(fwords[f++]);
    int tn = std::stoi(fwords[f++]);
    ray_cast_test test;
    test.points = read_from_txt_file_struct<parray<point3d>>()(fwords, f, pn);
    test.triangles = read_from_txt_file_struct<parray<triangle>>()(fwords, f, tn);
    parray<point3d> ray_ends = read_from_txt_file_struct<parray<point3d>>()(swords, s);
    parray<ray<point3d>> rays(test.triangles.size());
    for (int i = 0; i < rays.size(); i++) {
      new (&rays[i]) ray<point3d>(ray_ends[2 * i], ray_ends[2 * i + 1] - point3d(0, 0, 0));
    }
    test.rays = rays;
    return test;
  }
};

template <>
struct write_to_txt_file_struct<ray_cast_test> {
  void operator()(std::ofstream& out, ray_cast_test& x) {
    assert(false);
  }
};

template <>
struct write_to_txt_files_struct<ray_cast_test> {
  void operator()(std::ofstream& ouf, std::ofstream& ous, ray_cast_test& x) {
    write_to_txt_file_struct<parray<point3d>>()(ouf, x.points);
    write_to_txt_file_struct<parray<triangle>>()(ouf, x.triangles);
    parray<point3d> ray_ends(2 * x.rays.size());
    for (int i = 0; i < x.rays.size(); i++) {
      ray_ends[2 * i] = x.rays[i].o;
      ray_ends[2 * i + 1] = x.rays[i].d + point3d(0, 0, 0);
    }
    write_to_txt_file_struct<parray<point3d>>()(ous, ray_ends);
  }
};
                                                
template <class Item>
Item read_from_txt_file(std::string file) {
  std::vector<std::string> words = string_to_words(read_string_from_txt(file));
  int p = 1;
  return read_from_txt_file_struct<Item>()(words, p);
}

template <>
std::string read_from_txt_file<std::string>(std::string file) {
  return read_string_from_txt(file);
}

template <class Item>
Item read_from_txt_files(std::string first, std::string second) {
  std::vector<std::string> fwords = string_to_words(read_string_from_txt(first));
  std::vector<std::string> swords = string_to_words(read_string_from_txt(second));
  int f = 1;
  int s = 1;
  return read_from_txt_files_struct<Item>()(fwords, f, swords, s);
}

template <class Item>
void write_to_txt_file(std::string file, Item& x) {
  std::ofstream out(file);
  out << "pbbs-pctl-converted" << std::endl;
  write_to_txt_file_struct<Item>()(out, x);
  out.close();
}

template <>
void write_to_txt_file<std::string>(std::string file, std::string& x) {
  std::ofstream out(file);
  write_to_txt_file_struct<std::string>()(out, x);
  out.close();
}

template <class Item>
void write_to_txt_files(std::string first, std::string second, Item& x) {
  std::ofstream ouf(first);
  ouf << "pbbs-pctl-converted" << std::endl;
  std::ofstream ous(second);
  ous << "pbbs-pctl-converted" << std::endl;
  write_to_txt_files_struct<Item>()(ouf, ous, x);
  ouf.close();
  ous.close();
}

} // end namespace
} // end namespace
} // end namespace

#endif