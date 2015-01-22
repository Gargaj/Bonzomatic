class CFFT
{
  CFFT();
  ~CFFT();
  
  bool Init();
  bool Deinit();
  
  bool GetFFT( float * samples );
  
}