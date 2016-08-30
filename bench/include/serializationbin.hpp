#include <string>
#include <cstring>
#include <fstream>
#include "geometrydata.hpp"
#include "teststructs.hpp"
#include "graph.hpp"

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
struct read_from_file_struct<Item*> {
  Item* operator()(std::ifstream& in, long size) const {
    Item* result = (Item*)malloc(sizeof(Item) * size);
    in.read(reinterpret_cast<char*>(result), sizeof(Item) * size);
    return result;
  }
};

template <class Item>
struct write_to_file_struct<Item*> {
  void operator()(std::ofstream& out, Item* items, long size) {
    out.write(reinterpret_cast<char*>(&size), sizeof(long));
    out.write(reinterpret_cast<char*>(items), sizeof(Item) * size);
  }
};

template <class Item>
struct read_from_file_struct<parray<Item>> {
  parray<Item> operator()(std::ifstream& in) const {
    long size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(long));
    parray<Item> result(size);
    in.read(reinterpret_cast<char*>(result.begin()), sizeof(Item) * size);
    return result;
  }
};

template <class Item>
struct write_to_file_struct<parray<Item>> {
  void operator()(std::ofstream& out, parray<Item>& a) {
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

template <class Point>
struct read_from_file_struct<triangles<Point>> {
  triangles<Point> operator()(std::ifstream& in) const {
    triangles<Point> t;
    in.read(reinterpret_cast<char*>(&t.num_points), sizeof(long));
    t.p = read_from_file_struct<Point*>()(in, t.num_points);
    in.read(reinterpret_cast<char*>(&t.num_triangles), sizeof(long));
    t.t = read_from_file_struct<triangle*>()(in, t.num_triangles); return t;
  }
};

template <class Point>
struct write_to_file_struct<triangles<Point>> {
  void operator()(std::ofstream& out, triangles<Point>& x) {
    write_to_file_struct<Point*>()(out, x.p, x.num_points);
    write_to_file_struct<triangle*>()(out, x.t, x.num_triangles);
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
    write_to_file_struct<parray<point3d>>()(out, test.points);
    write_to_file_struct<parray<triangle>>()(out, test.triangles);
    write_to_file_struct<parray<ray<point3d>>>()(out, test.rays);
  }
};

template <class intT>
struct read_from_file_struct<graph::graph<intT>> {
  graph::graph<intT> operator()(std::ifstream& in) {
    intT n, m;
    in.read(reinterpret_cast<char*>(&n), sizeof(intT));
    in.read(reinterpret_cast<char*>(&m), sizeof(intT));
    intT* degree = new intT[n];
    in.read(reinterpret_cast<char*>(degree), sizeof(intT) * n);
    intT* e = (intT*)malloc(sizeof(intT) * m);
    in.read(reinterpret_cast<char*>(e), sizeof(intT) * m);
    graph::vertex<intT>* v = (graph::vertex<intT>*)malloc(sizeof(graph::vertex<intT>) * n);
    int offset = 0;
    for (int i = 0; i < n; i++) {
      v[i] = graph::vertex<intT>(e + offset, degree[i]);
      offset += degree[i];
    }
    delete [] degree;
    return graph::graph<intT>(v, n, m, e);
  }
};

template <class intT>
struct write_to_file_struct<graph::graph<intT>> {
  void operator()(std::ofstream& out, graph::graph<intT>& graph) {
    out.write(reinterpret_cast<char*>(&graph.n), sizeof(intT));
    out.write(reinterpret_cast<char*>(&graph.m), sizeof(intT));
    intT* degree = new int[graph.n];
    for (int i = 0; i < graph.n; i++) {
      degree[i] = graph.V[i].degree;
    }
    out.write(reinterpret_cast<char*>(degree), sizeof(intT) * graph.n);
    delete [] degree;
    for (int i = 0; i < graph.n; i++) {
      out.write(reinterpret_cast<char*>(graph.V[i].Neighbors), sizeof(intT) * graph.V[i].degree);
    }
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