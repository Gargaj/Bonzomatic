#include <time.h>

namespace Timer
{
  timespec startTime;

  void Start()
  {
    clock_gettime(CLOCK_MONOTONIC, &startTime);
  }

  float GetTime()
  {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec -= startTime.tv_sec;
    ts.tv_nsec -= startTime.tv_nsec;
    double now = ts.tv_sec * 1e3 + ts.tv_nsec * 1e-6;
    return now;
  }
}
