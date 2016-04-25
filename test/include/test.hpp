
#ifdef USE_PASL_RUNTIME
#include "threaddag.hpp"
#include "native.hpp"
#include "pcmdline.hpp"
#endif

#ifdef PCTL_CILK_PLUS
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

#include "quickcheck.hpp"
#include "io.hpp"
#include "cmdline.hpp"

#ifndef _PBBS_PCTL_TEST_H_
#define _PBBS_PCTL_TEST_H_

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
    deepsea::cmdline::set(argc, argv);
#if defined(USE_PASL_RUNTIME)
    pasl::util::cmdline::set(argc, argv);
    pasl::sched::threaddag::init();
#endif
#if defined(USE_CILK_RUNTIME)
    // hack that seems to be required to initialize cilk runtime cleanly
    cilk_spawn seq_fib(2);
    cilk_sync;
    body();
#elif defined(USE_PASL_RUNTIME)
    pasl::sched::threaddag::launch(pasl::sched::native::new_multishot_by_lambda([&] { body(); }));
#else
    body();
#endif
#if defined(USE_PASL_RUNTIME)
    pasl::sched::threaddag::destroy();
#endif
  }
  
} // end namespace

#endif /*! _PBBS_PCTL_TEST_H_ */