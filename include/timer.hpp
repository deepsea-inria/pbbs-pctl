#include <chrono>

#ifndef _PCTL_TIMER_
#define _PCTL_TIMER_

namespace pasl {
namespace pctl {

class timer {
  double total_time;
  std::chrono::time_point<std::chrono::system_clock> start_time, end_time;
  
public:
  timer() {}

  void start() {
#ifdef TIME_MEASURE
    start_time = std::chrono::system_clock::now();
#endif
  }

  double end() {
#ifdef TIME_MEASURE
    end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    double d = duration.count();
    total_time += d;
    return d;
#endif
  }

  double end(std::string s) {
#ifdef TIME_MEASURE
    printf("exectime %s %.3lf\n", s.c_str(), end());
#endif
  }

  double get_time() {
    return total_time;
  }

  void clear() {
    total_time = 0;
  }
};

}
}
#endif /*! _PBBS_PCTL_TIMER_ */