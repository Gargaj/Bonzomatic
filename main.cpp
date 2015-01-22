#include "Renderer.h"

void main()
{
  RENDERER_SETTINGS settings;
  settings.nWidth = 1280;
  settings.nHeight = 720;
  settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;

  Renderer::Open( &settings );

  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();
    Renderer::EndFrame();
  }

  Renderer::Close();
}