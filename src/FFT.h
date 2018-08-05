typedef struct
{
  int nFFTsize;
  bool bLoopback;
} FFT_SETTINGS;

namespace FFT
{
  bool Open( FFT_SETTINGS * settings );
  bool GetFFT( float * samples );
  void Close();
}