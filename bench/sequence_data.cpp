#include "bench.hpp"
#include "cmdline.hpp"
#include "sequencedata.hpp"
#include "trigram_generator.hpp"
#include "rays_generator.hpp"
#include "loaders.hpp"
#include "delaunay.hpp"
using namespace pasl::pctl;
namespace cmdline = deepsea::cmdline;

template <class Item>
struct generate_struct {};

template <>
struct generate_struct<parray<int>> {
  parray<int> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "random") {
      return sequencedata::rand<int>(0, n);
    } else if (generator == "random_bounded") {
      int range = cmdline::parse_or_default<int>("m", n);
      return sequencedata::rand_int_range<int>(0, n, range);
    } else if (generator == "almost_sorted") {
      int nb_swaps = cmdline::parse_or_default<int>("nb_swaps", (int)std::sqrt(n));
      return sequencedata::almost_sorted<int>(0, n, nb_swaps);
    } else if (generator == "all_same") {
      int value = cmdline::parse_or_default<int>("value", 1234);
      return sequencedata::all_same<int>(n, value);
    } else if (generator == "exponential") {
      return sequencedata::exp_dist<int>(0, n);
    } else {
      assert(false);
    }
  }
};

template <>
struct generate_struct<parray<double>> {
  parray<double> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "random") {
      return sequencedata::rand<double>(0, n);
    } else if (generator == "almost_sorted") {
      int nb_swaps = cmdline::parse_or_default<int>("nb_swaps", (int)std::sqrt(n));
      return sequencedata::almost_sorted<double>(0, n, nb_swaps);
    } else if (generator == "all_same") {
      int value = cmdline::parse_or_default<int>("value", 1234);
      return sequencedata::all_same<double>(n, value);
    } else if (generator == "exponential") {
      return sequencedata::exp_dist<double>(0, n);
    } else {
      assert(false);
    }
  }
};

template <>
struct generate_struct<parray<char*>> {
  parray<char*> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "trigrams") {
      return trigram_words(0, n);
    } else {
      assert(false);
    }
  }
};

template <>
struct generate_struct<parray<point2d>> {
  parray<point2d> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<long>("n");
    if (generator == "in_circle") {
      return uniform2d(true, false, n);
    } else if (generator == "on_circle") {
      return uniform2d(false, true, n);
    } else if (generator == "in_square") {
      return uniform2d(false, false, n);
    } else if (generator == "kuzmin") {
      return plummer2d(n);
    } else {
      assert(false);
    }
  }
};

template <>
struct generate_struct<parray<point3d>> {
  parray<point3d> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "in_sphere") {
      return uniform3d<int, unsigned int>(true, false, n);
    } else if (generator == "on_sphere") {
      return uniform3d<int, unsigned int>(false, true, n);
    } else if (generator == "in_cube") {
      return uniform3d<int, unsigned int>(false, false, n);
    } else if (generator == "plummer") {
      return plummer3d<int, unsigned int>(n);
    } else {
      assert(false);
    }
  }
};

template <class Item>
struct generate_struct<parray<std::pair<Item, int>>> {
  parray<std::pair<Item, int>> operator()(std::string generator) {
    parray<Item> a = generate_struct<parray<Item>>()(generator);
    int range = cmdline::parse_or_default<int>("m2", a.size());
    return parray<std::pair<Item, int>>(a.size(), [&] (int i) {
      int v = prandgen::hash<int>(a.size() + i) % range;
      return make_pair(a[i], v);
    });
  }
};

template <class Item>
struct generate_struct<parray<std::pair<Item, int>*>> {
  parray<std::pair<Item, int>*> operator()(std::string generator) {
    parray<std::pair<Item, int>> a = generate_struct<parray<std::pair<Item, int>>>()(generator);
    return parray<std::pair<Item, int>*>(a.size(), [&] (int i) {
      return new std::pair<Item, int>(a[i]);
    });
  }
};

template <>
struct generate_struct<std::string> {
  std::string operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "trigrams") {
      return trigram_string(0, n);
    } else {
      assert(false);
    }
  }
};

template <class Point>
struct generate_struct<triangles<Point>> {
  triangles<Point> operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "delaunay_in_square") {
      parray<point2d> points = uniform2d(false, false, n);
      return delaunay(points);
    } else if (generator == "delaunay_kuzmin") {
      parray<point2d> points = plummer2d(n);
      return delaunay(points);
    } else {
      assert(false);
    }
  }
};

template <>
struct generate_struct<io::ray_cast_test> {
  io::ray_cast_test operator()(std::string generator) {
    size_t n = (size_t)cmdline::parse<int>("n");
    if (generator == "on_sphere") {
      return generate_ray_cast_test(n, true);
    } else if (generator == "in_cube") {
      io::ray_cast_test test = generate_ray_cast_test(n, false);
      std::cerr << "Return generate_struct: " << test.points.size() << " " << test.triangles.size() << " " << test.rays.size() << std::endl;
      return test;
//      return generate_ray_cast_test(n, false);
    } else {
      assert(false);
    }
  }
};

template <class Item1, class Item2>
struct generate_struct<std::pair<Item1, Item2>> {
  std::pair<int, int> operator()(std::string generator) {
    assert(false);
  }
};

template <class intT>
struct generate_struct<graph::graph<intT>> {
  graph::graph<intT> operator()(std::string generator) {
    assert(false);
  }
};
                        
template <class Item>
Item generate(std::string generator) {
  return generate_struct<Item>()(generator);
}

template <class Item>
void store(Item& xs, std::string outfile) {
  std::string base;
  std::string extension;
  io::parse_filename(outfile, base, extension);
  if (extension == "txt") {
    io::write_to_txt_file(outfile, xs);
  } else if (extension == "bin") {
    io::write_to_file(outfile, xs);
  } else {
    assert(false);
  }
}

template <class Item>
void store(Item& xs, std::string outfile, std::string outfile2) {
  std::string base;
  std::string extension;
  io::parse_filename(outfile, base, extension);
  if (extension == "txt") {
    io::write_to_txt_files(outfile, outfile2, xs);
  } else {
    assert(false);
  }
}

template <class Item>
void process() {
//  Item* result = new Item();
  Item* result = (Item*)malloc(sizeof(Item));
  std::memset(result, 0, sizeof(Item));
  std::string generator = cmdline::parse_or_default<std::string>("generator", "");
  std::string infile = cmdline::parse_or_default<std::string>("infile", "");
  std::string outfile = cmdline::parse_string("outfile");
  if (generator != "") {
    *result = generate<Item>(generator);
  } else if (infile != "") {
    std::string infile2 = cmdline::parse_or_default<std::string>("infile2", "");
    if (infile2 != "") {
      *result = io::load<Item>(infile, infile2);
    } else {
      *result = io::load<Item>(infile);
    }
  } else {
    assert(false);
  }

  std::string outfile2 = cmdline::parse_or_default<std::string>("outfile2", "");
  if (outfile2 != "")  {
    store(*result, outfile, outfile2);
  } else {
    store(*result, outfile);
  }
  free(result);
}

int main(int argc, char ** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    cmdline::dispatcher d;
    d.add("array_double", [&] {
      process<parray<double>>();
    });
    d.add("array_int", [&] {
      process<parray<int>>();
    });
    d.add("array_pair_int_int", [&] {
      process<parray<std::pair<int, int>>>();
    });
    d.add("array_string", [&] {
      process<parray<char*>>();
    });
    d.add("array_pair_string_int", [&] {
      process<parray<std::pair<char*, int>>>();
    });
    d.add("array_point2d", [&] {
      process<parray<point2d>>();
    });
    d.add("array_point3d", [&] {
      process<parray<point3d>>();
    });
    d.add("string", [&] {
      process<std::string>();
    });
    d.add("ray_cast_test", [&] {
      process<io::ray_cast_test>();
    });
    d.add("pair_int_int", [&] {
      process<std::pair<int, int>>();
    });
    d.add("triangles_point2d", [&] {
      process<triangles<point2d>>();
    });
    d.add("graph", [&] {
      process<graph::graph<int>>();});
    d.dispatch("type");
  });
  return 0;
}