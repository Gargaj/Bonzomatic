#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#define BZC_CLOCK_MONOTONIC (0)
// clock_gettime is not implemented on OSX
int clock_gettime(int /*clk_id*/, struct timespec* ts) {
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
  return 0;
}
#else
#define BZC_CLOCK_MONOTONIC CLOCK_MONOTONIC
#endif

namespace Timer
{
  timespec startTime;

  void Start()
  {
    clock_gettime(BZC_CLOCK_MONOTONIC, &startTime);
  }

  float GetTime()
  {
    timespec ts;
    clock_gettime(BZC_CLOCK_MONOTONIC, &ts);
    ts.tv_sec -= startTime.tv_sec;
    ts.tv_nsec -= startTime.tv_nsec;
    double now = ts.tv_sec * 1e3 + ts.tv_nsec * 1e-6;
    return now;
  }
}
