

#ifndef _PBBS_PCTL_EXAMPLE_H_
#define _PBBS_PCTL_EXAMPLE_H_
#include "cmdline.hpp"
#ifdef PCTL_CILK_PLUS
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif
#include <iostream>
#include <string>

namespace pbbs {
  
#ifdef USE_CILK_RUNTIME
  static long seq_fib (long n){
    if (n < 2)
      return n;
    else
      return seq_fib (n - 1) + seq_fib (n - 2);
  }
#endif
  
template <class Body>
void launch(int argc, char** argv, const Body& body) {
  pasl::util::cmdline::set(argc, argv);
  int proc = pasl::util::cmdline::parse_or_default_int("proc", 1);

#if defined(USE_PASL_RUNTIME)
  threaddag::init();
#endif
#if defined(USE_CILK_RUNTIME)
  // hack that seems to be required to initialize cilk runtime cleanly
  cilk_spawn seq_fib(2);
  cilk_sync;
  body();
#elif defined(USE_PASL_RUNTIME)
  threaddag::launch(native::new_multishot_by_lambda([&] { body(); }));
#else
#ifdef PCTL_CILK_PLUS
  __cilkrts_set_param("nworkers", std::to_string(proc).c_str());
  std::cerr << "Number of workers: " << __cilkrts_get_nworkers() << std::endl;
#endif
  body();
#endif
#if defined(USE_PASL_RUNTIME)
  threaddag::destroy();
#endif
}
  
} // end namespace

#endif /*! _PBBS_PCTL_EXAMPLE_H_ */