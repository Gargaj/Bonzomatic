#include <iostream>
#include <cstring>

#include "../FFT.h"

namespace FFT
{
  bool Open()
  {
    std::cerr << __FUNCTION__ << " NOT IMPLEMENTED" << std::endl;
    return true;
  }
  bool GetFFT( float * samples )
  {
    memset(samples, 0, FFT_SIZE * sizeof(*samples));
    return false;
  }
  void Close()
  {
  }
}
