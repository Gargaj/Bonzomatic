#include <stdio.h>
#include <string.h>
#include <Scintilla.h>
#include "Renderer.h"
#include "FFT.h"
#include "Timer.h"


char fragmentSource[65536] = "#version 430 core\n"
  "\n"
  "///////////////////////////////////////////////////////////////////////////////\n"
  "// shader inputs/outputs\n"
  "///////////////////////////////////////////////////////////////////////////////\n"
  "uniform float fGlobalTime; // in seconds\n"
  "uniform vec2 v2Resolution; // viewport resolution (in pixels) (1080p or 720p)\n"
  "uniform mat4 iMidiPad; // 16 buttons of midi controller\n"
  "uniform float iMidiPadValue; // sum of all elements in iMidiPad/16\n"
  "\n"
  "// all samplers have linear filtering applied, wraping set to repeat\n"
  "//\n"
  "uniform sampler1D texFFT; // 1024\n"
//  "uniform float iFFT[8]; // latest frame\n"
//  "uniform float iFFTs[8]; // smoothed latest frame\n"
//  "uniform sampler2D iFFTsHistory; // smoothed fft history, 8x1024, x coord = bin, y coord n-frames earlier, y=0 is latest frame\n"
  "\n"
  "// predefined textures\n"
  "//\n"
  "uniform sampler2D texTex1;\n"
  "uniform sampler2D texTex2;\n"
  "uniform sampler2D texTex3;\n"
  "uniform sampler2D texTex4;\n"
  "uniform sampler2D texNoise;\n"
  "uniform sampler2D texChecker;\n"
  "\n"
  "// out_color must be written in order to see anything\n"
  "//\n"
  "layout(location = 0) out vec4 out_color;\n"
  "///////////////////////////////////////////////////////////////////////////////\n"
  "///////////////////////////////////////////////////////////////////////////////\n"
  "\n"
  "void main(void)\n"
  "{\n"
  "	vec2 uv = vec2( gl_FragCoord.xy ) / v2Resolution;\n"
  "	vec2 uv5 = uv - 0.5;\n"
  "	vec2 m;\n"
  "	m.x = atan(uv5.x / uv5.y);\n"
  "	m.y = length(uv5);\n"
  "	float f = texture( texFFT, m.y  ).r * 1000;\n"
  "	m.y -= fGlobalTime;\n"
  "	vec4 t = texture( texChecker, m.xy  );\n"
  "	out_color = f + t;// + uv.xyxy * 0.5 * (sin( fGlobalTime ) + 1.5);\n"
  "}";

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
  Renderer::Texture * texNoise = Renderer::CreateRGBA8TextureFromFile("textures/noise.png");
  Renderer::Texture * tex[4];
  for(int i=0; i<4; i++)
  {
    char sz[] = "textures/tex1.png";
    sz[12] = '1' + i;
    tex[i] = Renderer::CreateRGBA8TextureFromFile(sz);
  }

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );

  char szError[4096];
  if (!Renderer::ReloadShader( fragmentSource, strlen(fragmentSource), szError, 4096 ))
  {
    return;
  }

  Scintilla_LinkLexers();

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

    for(int i=0; i<4; i++)
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