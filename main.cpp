#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <Scintilla.h>
#include <Platform.h>

#include <ILexer.h>

#ifdef SCI_LEXER
#include <SciLexer.h>
#endif
#include <StringCopy.h>
#ifdef SCI_LEXER
#include <LexerModule.h>
#endif
#include <SplitVector.h>
#include <Partitioning.h>
#include <RunStyles.h>
#include <ContractionState.h>
#include <CellBuffer.h>
#include <CallTip.h>
#include <KeyMap.h>
#include <Indicator.h>
#include <XPM.h>
#include <LineMarker.h>
#include <Style.h>
#include <ViewStyle.h>
#include <CharClassify.h>
#include <Decoration.h>
#include <CaseFolder.h>
#include <Document.h>
#include <CaseConvert.h>
#include <UniConversion.h>
#include <Selection.h>
#include <PositionCache.h>
#include <EditModel.h>
#include <MarginView.h>
#include <EditView.h>
#include <Editor.h>

#ifdef SCI_LEXER
#include <ExternalLexer.h>
#endif

#include "Renderer.h"
#include "FFT.h"
#include "Timer.h"

/*
class MyEditor : public Scintilla::Editor
{
};
*/

void main()
{
  //MyEditor mShaderEditor;

  RENDERER_SETTINGS settings;
  settings.nWidth = 1280;
  settings.nHeight = 720;
  settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;

  if (!Renderer::Open( &settings ))
    return;

  if (!FFT::Open())
    return;

  printf("Loading textures...\n");

  printf("* textures/checker.png...\n");
  Renderer::Texture * texChecker = Renderer::CreateRGBA8TextureFromFile("textures/checker.png");

  printf("* textures/noise.png...\n");
  Renderer::Texture * texNoise = Renderer::CreateRGBA8TextureFromFile("textures/noise.png");

  Renderer::Texture * tex[8];
  for(int i=0; i<8; i++)
  {
    char sz[] = "textures/tex1.jpg";
    sz[12] = '1' + i;
    printf("* %s...\n",sz);
    tex[i] = Renderer::CreateRGBA8TextureFromFile(sz);
  }

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );

  char szError[4096];
  if (!Renderer::ReloadShader( Renderer::defaultShader, strlen(Renderer::defaultShader), szError, 4096 ))
  {
    return;
  }

#ifdef SCI_LEXER
  Scintilla_LinkLexers();
#endif

  Timer::Start();
  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();

    float time = Timer::GetTime() / 1000.0; // seconds
    Renderer::SetShaderConstant( "fGlobalTime", time );

    Renderer::SetShaderConstant( "v2Resolution", settings.nWidth, settings.nHeight );

    Renderer::SetShaderTexture( "texChecker", texChecker );
    Renderer::SetShaderTexture( "texNoise", texNoise );

    static float fftData[FFT_SIZE];
    if (FFT::GetFFT(fftData))
      Renderer::UpdateR32Texture( texFFT, fftData );

    Renderer::SetShaderTexture( "texFFT", texFFT );

    for(int i=0; i<8; i++)
    {
      char sz[] = "texTex1";
      sz[6] = '1' + i;
      Renderer::SetShaderTexture( sz, tex[i] );
    }

    Renderer::RenderFullscreenQuad();

    Renderer::EndFrame();
  }

  FFT::Close();

  Renderer::Close();
}