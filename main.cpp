#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ShaderEditor.h"
#include "Renderer.h"
#include "FFT.h"
#include "Timer.h"
#include "external\scintilla\src\UniConversion.h"

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

  bool shaderInitSuccessful = false;
  char szShader[65535];
  char szError[4096];
  FILE * f = fopen("shader.fs","rb");
  if (f)
  {
    memset( szShader, 0, 65535 );
    int n = fread( szShader, 1, 65535, f );
    fclose(f);
    if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
    {
      shaderInitSuccessful = true;
    }
  }
  if (!shaderInitSuccessful)
  {
    memcpy( szShader, Renderer::defaultShader, 65535 );
    if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
    {
      assert(0);
    }
  }

#ifdef SCI_LEXER
  Scintilla_LinkLexers();
#endif
  Scintilla::Surface * surface = Scintilla::Surface::Allocate( SC_TECHNOLOGY_DEFAULT );
  surface->Init( NULL );

  ShaderEditor mShaderEditor( surface );
  mShaderEditor.Initialise();
  mShaderEditor.SetText( szShader );

  bool bShowGui = true;
  Timer::Start();
  float fNextTick = 0.1;
  while (!Renderer::WantsToQuit())
  {
    Renderer::StartFrame();

    for(int i=0; i<Renderer::keyEventBufferCount; i++)
    {
      if (Renderer::keyEventBuffer[i].scanCode == 286) // F5
      {
        mShaderEditor.GetText(szShader,65535);
        if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
        {
          FILE * f = fopen("shader.fs","wb");
          fwrite( szShader, strlen(szShader), 1, f );
          fclose(f);
        }
        else
        {
          // show error
        }
      }
      else if (Renderer::keyEventBuffer[i].scanCode == 292) // F11
      {
        bShowGui = !bShowGui;
      }
      else
      {
        bool consumed = false;
        mShaderEditor.KeyDown(
          iswalpha(Renderer::keyEventBuffer[i].scanCode) ? towupper(Renderer::keyEventBuffer[i].scanCode) : Renderer::keyEventBuffer[i].scanCode,
          Renderer::keyEventBuffer[i].shift,
          Renderer::keyEventBuffer[i].ctrl, 
          Renderer::keyEventBuffer[i].alt,
          &consumed);
        if (!consumed)
        {
          char    utf8[5] = {0,0,0,0,0};
          wchar_t utf16[2] = {Renderer::keyEventBuffer[i].character, 0};
          Scintilla::UTF8FromUTF16(utf16, 1, utf8, 4 * sizeof(char));
          mShaderEditor.AddCharUTF(utf8, strlen(utf8));
        }

      }
    }
    Renderer::keyEventBufferCount = 0;
    //mShaderEditor.KeyDown();

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

    if (bShowGui)
    {
      if (time > fNextTick)
      {
        mShaderEditor.Tick();
        fNextTick = time + 0.1;
      }

      mShaderEditor.Paint( );
    }

    Renderer::EndFrame();
  }

  delete surface;

  FFT::Close();

  Renderer::Close();
}