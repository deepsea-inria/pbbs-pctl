
#include <iostream>
#include <chrono>

#include "io.hpp"
#include "cmdline.hpp"

#ifndef _PBBS_PCTL_BENCH_H_
#define _PBBS_PCTL_BENCH_H_

namespace pbbs {
  
#ifdef USE_CILK_RUNTIME
  static long seq_fib (long n){
    if (n < 2)
      return n;
    else
      return seq_fib (n - 1) + seq_fib (n - 2);
  }
#endif
  
  namespace {
    
    template <class Body>
    void launch(const Body& body) {
    #if defined(USE_CILK_RUNTIME)
      // hack that seems to be required to initialize cilk runtime cleanly
      cilk_spawn seq_fib(2);
      cilk_sync;
      body();
    #elif defined(USE_PASL_RUNTIME)
      threaddag::launch(native::new_multishot_by_lambda(body));
    #else
      body();
    #endif
    }
    
  } // end namespace
  
  using thunk_type = std::function<void()>;
  using measured_type = std::function<void(thunk_type)>;
  
  template <class Body>
  void launch(int argc, char** argv, const Body& body) {
    pasl::util::cmdline::set(argc, argv);
#if defined(USE_PASL_RUNTIME)
    threaddag::init();
#endif
    auto f = [&] (thunk_type measured) {
      auto start = std::chrono::system_clock::now();
      measured();
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<float> diff = end - start;
      printf ("exectime %.3lf\n", diff.count());
    };
    launch([&] {
      body(f);
    });
#if defined(USE_PASL_RUNTIME)
    threaddag::destroy();
#endif
  }
  
} // end namespace

#endif /*! _PBBS_PCTL_BENCH_H_ */