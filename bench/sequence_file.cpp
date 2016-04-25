#include "bench.hpp"
#include "cmdline.hpp"
#include "sequencedata.hpp"
#include "ioshared.hpp"

using namespace pasl::pctl;
namespace cmdline = deepsea::cmdline;

template <class Item>
parray<Item> generate(std::string generator) {
  parray<Item> result;
  size_t n = (size_t)cmdline::parse<long>("n");
  if (generator == "random") {
    result = sequencedata::rand<Item>(0, n);
  } else if (generator == "almost_sorted") {
    result = sequencedata::almost_sorted<Item>(0, n, n/2);
  } else if (generator == "all_same") {
    result = sequencedata::all_same<Item>(n, 1234);
  } else if (generator == "exponential") {
    result = sequencedata::exp_dist<Item>(0, n);
  } else {
    assert(false);
  }
  return result;
}

template <class Item>
parray<Item> load(std::string infile) {
  return io::read_seq_from_txt<Item>(infile);
}

template <class Item>
void store(const parray<Item>& xs, std::string outfile) {
  assert(false); // todo
}

template <class Item>
void process() {
  parray<Item> result;
  std::string generator = cmdline::parse_or_default("generator", "");
  std::string infile = cmdline::parse_or_default("infile", "");
  std::string outfile = cmdline::parse_string("outfile");
  if (generator != "") {
    result = generate<Item>(generator);
  } else if (infile != "") {
    result = load<Item>(infile);
  } else {
    assert(false);
  }
  store(result, outfile);
}

int main(int argc, char ** argv) {
  pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
    cmdline::dispatcher d;
    d.add("double", [&] {
      process<double>();
    });
    d.add("string", [&] {
      assert(false); // todo
    });
    d.dispatch("type");
  });
  return 0;
}