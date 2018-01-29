
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <limits.h>

#include "io.hpp"
#include "cmdline.hpp"
#include "granularity.hpp"

#ifdef USE_PASL_RUNTIME
#include "threaddag.hpp"
#include "native.hpp"
#include "pcmdline.hpp"
#endif

#ifdef HAVE_HWLOC
#include <hwloc.h>
#endif

#ifdef PCTL_CILK_PLUS
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

#ifndef _PBBS_PCTL_BENCH_H_
#define _PBBS_PCTL_BENCH_H_

namespace pbbs {

#ifdef HAVE_HWLOC
hwloc_topology_t    topology;
#endif
  
#ifdef USE_CILK_RUNTIME
  static long seq_fib (long n){
    if (n < 2)
      return n;
    else
      return seq_fib (n - 1) + seq_fib (n - 2);
  }
#endif
  
  namespace {

    void load_granularity_parameters() {
      char _hostname[HOST_NAME_MAX];
      gethostname(_hostname, HOST_NAME_MAX);
      std::string hostname = std::string(_hostname);
      if (hostname == "teraram") {
	pasl::pctl::granularity::kappa = 25.0;
	pasl::pctl::granularity::update_size_ratio = 1.5;
      } else if (hostname == "cadmium") {
	pasl::pctl::granularity::kappa = 30.0;
	pasl::pctl::granularity::update_size_ratio = 1.1;
      } else if (hostname == "hiphi.aladdin.cs.cmu.edu") {
	pasl::pctl::granularity::kappa = 40.0;
	pasl::pctl::granularity::update_size_ratio = 1.2;
      } else if (hostname == "aware.aladdin.cs.cmu.edu") {
	pasl::pctl::granularity::kappa = 40.0;
	pasl::pctl::granularity::update_size_ratio = 1.2;
      }


      pasl::pctl::granularity::kappa =
	deepsea::cmdline::parse_or_default_double("kappa", pasl::pctl::granularity::kappa);
      pasl::pctl::granularity::update_size_ratio =
	deepsea::cmdline::parse_or_default_double("alpha", pasl::pctl::granularity::update_size_ratio);
    }
    
    template <class Body>
    void launch(const Body& body) {  
    #if defined(USE_CILK_RUNTIME)
      // hack that seems to be required to initialize cilk runtime cleanly
      cilk_spawn seq_fib(2);
      cilk_sync;
      __cilkg_take_snapshot_for_stats();
      body();
      /* recall: if using the custom cilk runtime, need to set the
       * environment variable as such:
       *   export LD_LIBRARY_PATH=/home/rainey/cilk-plus-rts/lib:$LD_LIBRARY_PATH
       */
      __cilkg_dump_encore_stats_to_stderr();
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
    load_granularity_parameters();
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
#if defined(HAVE_HWLOC) && defined(USE_CILK_PLUS_RUNTIME)
  hwloc_topology_init (&topology);
  hwloc_topology_load (topology);
  bool numa_alloc_interleaved = (proc == 1) ? false : true;
  numa_alloc_interleaved =
    deepsea::cmdline::parse_or_default_bool("numa_alloc_interleaved", numa_alloc_interleaved, false);
  if (numa_alloc_interleaved) {
    hwloc_cpuset_t all_cpus =
      hwloc_bitmap_dup (hwloc_topology_get_topology_cpuset (topology));
    int err = hwloc_set_membind(topology, all_cpus, HWLOC_MEMBIND_INTERLEAVE, 0);
    if (err < 0)
      printf("Warning: failed to set NUMA round-robin allocation policy\n");
  }
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
#if defined(PLOGGING) || defined(THREADS_CREATED)
    printf("number of cstmt calls: %d\n", pasl::pctl::granularity::calls_created());
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
