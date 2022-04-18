namespace SetupDialog
{
  typedef struct 
  {
    RENDERER_SETTINGS sRenderer;
    FFT_SETTINGS sFFT;
  } SETTINGS;

  bool Open( SETTINGS * pSettings );
}