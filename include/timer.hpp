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
    start_time = std::chrono::system_clock::now();
  }

  void end() {
    end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    total_time += duration.count();
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