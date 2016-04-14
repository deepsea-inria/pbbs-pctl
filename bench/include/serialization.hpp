#include <string>
#include <fstream>
#include "geometrydata.hpp"

#ifndef _PCTL_PBBS_SERIALIZATION_H_
#define _PCTL_PBBS_SERIALIZATION_H_

namespace pasl {
namespace pctl {
namespace io {

template <class Item>
struct read_from_file {
  Item operator()(std::ifstream& in) const {
    Item memory;
    in.read(reinterpret_cast<char*>(&memory), sizeof(Item));
    return memory;
  }
};

template <class Item>
void write_to_file(std::ofstream& out, Item& item) {
  out.write(reinterpret_cast<char*>(&item), sizeof(Item));
}

template <>
struct read_from_file<std::string> {
  std::string operator()(std::ifstream& in) const {
    int size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(int));
    std::string answer;
    answer.reserve(size);
    in.read(&answer[0], sizeof(char) * size);
    return answer;
  }
};

template <>
void write_to_file<std::string>(std::ofstream& out, std::string& s) {
  int size = s.size();
  out.write(reinterpret_cast<char*>(&size), sizeof(int));
  out.write(&s[0], sizeof(char) * size);
}

template <class Item>
struct read_from_file<parray<Item>> {
  parray<Item> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    parray<Item> result(size);
    in.read(reinterpret_cast<char*>(result.begin()), sizeof(Item) * size);
  }
};

template <class Item>
void write_to_file(std::ofstream& out, parray<Item>& a) {
  int size = a.size();
  out.write(reinterpret_cast<char*>(&size), sizeof(long));
  out.write(reinterpret_cast<char*>(a.begin()), sizeof(Item) * size);
}

template <>
struct read_from_file<parray<std::string>> {
  parray<std::string> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    parray<std::string> result(size);
    int* len = new int[size];
    in.read(reinterpret_cast<char*>(len), sizeof(int) * size);
    for (int i = 0; i < size; i++) {
      result[i].reserve(len[i]);
      in.read(&result[i][0], sizeof(char) * len[i]);
    }
    delete [] len;
  }
};

template <>
void write_to_file<parray<std::string>>(std::ofstream& out, parray<std::string>& a) {
  long size = a.size();
  out.write(reinterpret_cast<char*>(&size), sizeof(long));
  int* len = new int[size];
  int total = 0;
  for (int i = 0; i < size; i++) {
    len[i] = a[i].length();
  }
  out.write(reinterpret_cast<char*>(len), sizeof(int) * size);
  for (int i = 0; i < size; i++) {
    out.write(&a[i][0], sizeof(char) * len[i]);
  }
  delete [] len;
}

template <>
struct read_from_file<std::pair<std::string, int>> {
  parray<std::pair<std::string, int>> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    int* len = new int[size];
    in.read(reinterpret_cast<char*>(len), sizeof(int) * size);
    parray<std::pair<std::string, int>> result(size);
    for (int i = 0; i < size; i++) {
      result[i].first.reserve(len[i]);
      in.read(&result[i].first[0], sizeof(char) * len[i]);
      in.read(reinterpret_cast<char*>(&result[i].second), sizeof(int));
    }
    delete [] len;
  }
};

template <>
void write_to_file<parray<std::pair<std::string, int>>>(std::ofstream& out, parray<std::pair<std::string, int>>& a) {
  long size = a.size();
  out.write(reinterpret_cast<char*>(&size), sizeof(long));
  int* len = new int[size];
  int total = 0;
  for (int i = 0; i < size; i++) {
    len[i] = a[i].first.length();
  }
  out.write(reinterpret_cast<char*>(len), sizeof(int) * size);
  for (int i = 0; i < size; i++) {
    out.write(&a[i].first[0], sizeof(char) * len[i]);
    out.write(reinterpret_cast<char*>(&a[i].second), sizeof(int));
  }
  delete [] len;
}

class ray_cast_test {
public:
  parray<point3d> points;
  parray<triangle> triangles;
  parray<ray<point3d>> rays;
};

template <>
struct read_from_file<ray_cast_test> {
  ray_cast_test operator()(std::ifstream& in) {
    ray_cast_test test;
  
    test.points = read_from_file<parray<point3d>>()(in);
    test.triangles = read_from_file<parray<triangle>>()(in);
    test.rays = read_from_file<parray<ray<point3d>>>()(in);
  
    return test;
  }
};

template <>
void write_to_file<ray_cast_test>(std::ofstream& out, ray_cast_test& test) {
  write_to_file<parray<point3d>>(out, test.points);
  write_to_file<parray<triangle>>(out, test.triangles);
  write_to_file<parray<ray<point3d>>>(out, test.rays);
}

} //end namespace
} //end namespace
} //end namespace

#endif /*! _PCTL_PBBS_SERIALIZATION_H_ */