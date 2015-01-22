#include "Renderer.h"
#include "FFT.h"

void main()
{
  RENDERER_SETTINGS settings;
  settings.nWidth = 1280;
  settings.nHeight = 720;
  settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;

  if (!Renderer::Open( &settings ))
    return;

  if (!FFT::Open())
    return;

  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();

    float fftData[FFT_SIZE];
    FFT::GetFFT(fftData);

    Renderer::EndFrame();
  }

  FFT::Close();

  Renderer::Close();
}