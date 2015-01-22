#include "Renderer.h"
#include "FFT.h"
#include "Timer.h"
#include <Scintilla.h>

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

  Renderer::Texture * texChecker = Renderer::CreateRGBA8TextureFromFile("textures/checker.png");

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );

  Scintilla_LinkLexers();

  Timer::Start();
  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();

    float time = Timer::GetTime();
    Renderer::SetShaderConstant( "fGlobalTime", time );

    Renderer::SetShaderConstant( "v2Resolution", settings.nWidth, settings.nHeight );

    Renderer::SetShaderTexture( "texChecker", texChecker );

    static float fftData[FFT_SIZE];
    FFT::GetFFT(fftData);
    Renderer::UpdateR32Texture( texFFT, fftData );

    Renderer::SetShaderTexture( "texFFT", texFFT );

    Renderer::EndFrame();
  }

  FFT::Close();

  Renderer::Close();
}