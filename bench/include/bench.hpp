
#include <iostream>
#include <chrono>

#include "io.hpp"
#include "cmdline.hpp"
#include "granularity.hpp"

#ifdef USE_PASL_RUNTIME
#include "threaddag.hpp"
#include "native.hpp"
#include "pcmdline.hpp"
#endif

#ifdef PCTL_CILK_PLUS
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

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
      pasl::sched::threaddag::launch(pasl::sched::native::new_multishot_by_lambda(body));
    #else
      body();
    #endif
    }
    
  } // end namespace
  
  using thunk_type = std::function<void()>;
  using measured_type = std::function<void(thunk_type)>;
  
  template <class Body>
  void launch(int argc, char** argv, const Body& body) {
    deepsea::cmdline::set(argc, argv);
    pasl::pctl::callback::init();
    pasl::pctl::granularity::try_read_constants_from_file();

#if defined(USE_PASL_RUNTIME)
    pasl::util::cmdline::set(argc, argv);
    pasl::sched::threaddag::init();
    LOG_BASIC(ENTER_ALGO);
#endif
#ifdef USE_CILK_PLUS_RUNTIME
    int proc = deepsea::cmdline::parse_or_default_int("proc", 1);
  __cilkrts_set_param("nworkers", std::to_string(proc).c_str());
  std::cerr << "Number of workers: " << __cilkrts_get_nworkers() << std::endl;pasl::pctl::granularity::nb_proc = proc;
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
#ifdef REPORTS
    pasl::pctl::granularity::print_reports();
#endif
#ifdef PLOGGING
    pasl::pctl::logging::dump();
#endif
#if defined(PLOGGING) || defined(THREADS_CREATED)
    printf("number of created threads: %d\n", pasl::pctl::granularity::threads_created());
#endif
#if defined(USE_PASL_RUNTIME)
    STAT_IDLE(sum());
    STAT(dump(stdout));
    STAT_IDLE(print_idle(stdout));
    LOG_BASIC(EXIT_ALGO);
    pasl::sched::threaddag::destroy();
#endif
    pasl::pctl::callback::output();
    pasl::pctl::granularity::try_write_constants_to_file();
    pasl::pctl::callback::destroy();
  }
  
} // end namespace

#endif /*! _PBBS_PCTL_BENCH_H_ */
