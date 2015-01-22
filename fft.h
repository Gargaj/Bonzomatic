#define FFT_SIZE 1024

namespace FFT
{
  bool Open();
  bool GetFFT( float * samples );
  void Close();
}