#include <string>
#include <cstring>
#include <fstream>
#include "geometrydata.hpp"
#include "teststructs.hpp"

#ifndef _PCTL_PBBS_SERIALIZATION_H_     
#define _PCTL_PBBS_SERIALIZATION_H_

namespace pasl {
namespace pctl {
namespace io {

template <class Item>
Item read_from_file(std::ifstream& in);

template <class Item>
struct read_from_file_struct {
  Item operator()(std::ifstream& in) const {
    Item memory;
    in.read(reinterpret_cast<char*>(&memory), sizeof(Item));
    return memory;
  }
};

template <class Item>
struct write_to_file_struct {
  void operator()(std::ofstream& out, Item& item) {
    out.write(reinterpret_cast<char*>(&item), sizeof(Item));
  }
};

template <>
struct read_from_file_struct<std::string> {
  std::string operator()(std::ifstream& in) const {
    int size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(int));
    std::string answer;
    answer.resize(size);
    in.read(&answer[0], size);
    return answer;
  }
};

template <>
struct write_to_file_struct<std::string> {
  void operator()(std::ofstream& out, std::string& s) {
    int size = s.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(int));
    out.write(&s[0], sizeof(char) * size);
  }
};

template <class Item>
struct read_from_file_struct<parray<Item>> {
  parray<Item> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    std::cerr << "Read size: " << size << std::endl;
    parray<Item> result(size);
    in.read(reinterpret_cast<char*>(result.begin()), sizeof(Item) * size);
    return result;
  }
};

template <class Item>
struct write_to_file_struct<parray<Item>> {
  void operator()(std::ofstream& out, parray<Item>& a) {
//    std::cerr << "Write to file\n";
//    std::cerr << a.size() << std::endl;
    long size = a.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(long));
    out.write(reinterpret_cast<char*>(a.begin()), sizeof(Item) * size);
  }
};

template <>
struct read_from_file_struct<parray<char*>> {
  parray<char*> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    parray<char*> result(size);
    int* len = new int[size];
    in.read(reinterpret_cast<char*>(len), sizeof(int) * size);
    for (int i = 0; i < size; i++) {
      result[i] = new char[len[i] + 1];
      result[i][len[i]] = 0;
      in.read(&result[i][0], sizeof(char) * len[i]);
    }
    delete [] len;
    return result;
  }
};

template <>
struct write_to_file_struct<parray<char*>> {
  void operator()(std::ofstream& out, parray<char*>& a) {
    long size = a.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(long));
    int* len = new int[size];
    int total = 0;
    for (int i = 0; i < size; i++) {
      len[i] = std::strlen(a[i]);
    }
    out.write(reinterpret_cast<char*>(len), sizeof(int) * size);
    for (int i = 0; i < size; i++) {
      out.write(&a[i][0], sizeof(char) * len[i]);
    }
    delete [] len;
  }
};

template <>
struct read_from_file_struct<parray<std::pair<char*, int>*>> {
  parray<std::pair<char*, int>*> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    int* len = new int[size];
    in.read(reinterpret_cast<char*>(len), sizeof(int) * size);
    parray<std::pair<char*, int>*> result(size);
    for (int i = 0; i < size; i++) {
      char* f = new char[len[i] + 1];
      f[len[i]] = 0;
      in.read(f, sizeof(char) * len[i]);
      int s = 0;
      in.read(reinterpret_cast<char*>(&s), sizeof(int));
      result[i] = new std::pair<char*, int>(f, s);
    }
    delete [] len;
    return result;
  }
};

template <>
struct write_to_file_struct<parray<std::pair<char*, int>*>> {
  void operator()(std::ofstream& out, parray<std::pair<char*, int>*>& a) {
    long size = a.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(long));
    int* len = new int[size];
    int total = 0;
    for (int i = 0; i < size; i++) {
      len[i] = std::strlen(a[i]->first);
    }
    out.write(reinterpret_cast<char*>(len), sizeof(int) * size);
    for (int i = 0; i < size; i++) {
      out.write(&a[i]->first[0], sizeof(char) * len[i]);
      out.write(reinterpret_cast<char*>(&a[i]->second), sizeof(int));
    }
    delete [] len;
  }
};

template <>
struct read_from_file_struct<ray_cast_test> {
  ray_cast_test operator()(std::ifstream& in) {
    ray_cast_test test;
  
    test.points = read_from_file<parray<point3d>>(in);
    test.triangles = read_from_file<parray<triangle>>(in);
    test.rays = read_from_file<parray<ray<point3d>>>(in);
  
    return test;
  }
};

template <>
struct write_to_file_struct<ray_cast_test> {
  void operator()(std::ofstream& out, ray_cast_test& test) {
//    std::cerr << "Write to file " << test.points.size() << " " << test.triangles.size() << " " << test.rays.size() << std::endl;
    write_to_file_struct<parray<point3d>>()(out, test.points);
    write_to_file_struct<parray<triangle>>()(out, test.triangles);
    write_to_file_struct<parray<ray<point3d>>>()(out, test.rays);
  }
};

template <class Item>
Item read_from_file(std::ifstream& in) {
  return read_from_file_struct<Item>()(in);
}

template <class Item>
void write_to_file(std::ofstream& out, Item& item) {
  write_to_file_struct<Item>()(out, item);
}

template <class Item>
Item read_from_file(std::string file) {
  std::ifstream in(file, std::ifstream::binary);
  return read_from_file<Item>(in);
}

template <class Item>
void write_to_file(std::string file, Item& x) {
  std::ofstream out(file, std::ofstream::binary);
  write_to_file(out, x);
}

} //end namespace
} //end namespace
} //end namespace

#endif /*! _PCTL_PBBS_SERIALIZATION_H_ */