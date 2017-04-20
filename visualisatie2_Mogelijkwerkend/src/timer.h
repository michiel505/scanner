#ifndef _TIMER_H_
#define _TIMER_H_

#include "libs.h"

class Timer // Precision of microseconds
{
private:
  std::chrono::high_resolution_clock::time_point m_tp;

public:
  Timer() {}
  ~Timer() {}

  void Reset() { m_tp = std::chrono::high_resolution_clock::now(); }

  int64_t GetElapsed() {
    std::chrono::microseconds elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock::now() - m_tp);
    return elapsed.count();
  }

  void Wait(const int64_t microseconds) {
    std::chrono::microseconds elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock::now() - m_tp);
    if (elapsed.count() < microseconds) {
      usleep(microseconds - elapsed.count());
    }
  }

  bool TimeOut(const int64_t micros_span) {
    std::chrono::microseconds elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::high_resolution_clock::now() - m_tp);
    if (elapsed.count() >= micros_span) {
      return true;
    }
    return false;
  }
};

#endif // _TIMER_H_
