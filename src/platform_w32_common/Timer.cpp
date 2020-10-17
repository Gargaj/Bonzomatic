#include <windows.h>

namespace Timer
{
  LARGE_INTEGER LastPCV = { 0 } ;
  double currentTime = 0.0;
  double startTime = 0.0;

  double _Time()
  {
    LARGE_INTEGER count, freq;
    if (!LastPCV.QuadPart) {
      QueryPerformanceCounter(&LastPCV);
    }
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);

    currentTime += (double)(count.QuadPart - LastPCV.QuadPart) / (double)(freq.QuadPart);

    LastPCV=count;

    return currentTime * 1000.0f;
  }

  void Start()
  {
    startTime = (float)_Time();
  }

  float GetTime()
  {
    return (float) ( _Time() - startTime );
  }
}