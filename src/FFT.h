#define FFT_SIZE 1024

namespace FFT
{
  extern float fAmplification;

  bool Open();
  bool GetFFT( float * _samples );
  void Close();
}