#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ShaderEditor.h"
#include "Renderer.h"
#include "FFT.h"
#include "MIDI.h"
#include "Timer.h"
#include "Misc.h"
#include "UniConversion.h"
#include "jsonxx.h"
#include "Capture.h"

#ifdef WIN32
#include <windows.h>
#endif

void ReplaceTokens( std::string &sDefShader, const char * sTokenBegin, const char * sTokenName, const char * sTokenEnd, std::vector<std::string> &tokens )
{
  if (sDefShader.find(sTokenBegin) != std::string::npos
    && sDefShader.find(sTokenName) != std::string::npos
    && sDefShader.find(sTokenEnd) != std::string::npos
    && sDefShader.find(sTokenBegin) < sDefShader.find(sTokenName)
    && sDefShader.find(sTokenName) < sDefShader.find(sTokenEnd))
  {
    int nTokenStart = sDefShader.find(sTokenBegin) + strlen(sTokenBegin);
    std::string sTextureToken = sDefShader.substr( nTokenStart, sDefShader.find(sTokenEnd) - nTokenStart );

    std::string sFinalShader;
    sFinalShader = sDefShader.substr( 0, sDefShader.find(sTokenBegin) );

    //for (std::map<std::string, Renderer::Texture*>::iterator it = tokens.begin(); it != tokens.end(); it++)
    for (int i=0; i < tokens.size(); i++)
    {
      std::string s = sTextureToken;
      while (s.find(sTokenName) != std::string::npos)
      {
        s.replace( s.find(sTokenName), strlen(sTokenName), tokens[i], 0, std::string::npos );
      }
      sFinalShader += s;
    }
    sFinalShader += sDefShader.substr( sDefShader.find(sTokenEnd) + strlen(sTokenEnd), std::string::npos );
    sDefShader = sFinalShader;
  }
}

#ifdef __APPLE__
#include <sys/param.h> // For MAXPATHLEN
#include "CoreFoundation/CoreFoundation.h"
static void changeToAppsCurrentDirectory()
{
  char appPath[MAXPATHLEN];
  CFBundleRef bundle = CFBundleGetMainBundle();
  if (!bundle) return;

  CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
  CFURLRef pathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);
  if (!CFURLGetFileSystemRepresentation(pathURL, true, (UInt8*)appPath, MAXPATHLEN))
  {
    CFRelease(bundleURL);
    CFRelease(pathURL);
    return;
  }
  CFRelease(bundleURL);
  CFRelease(pathURL);

  chdir(appPath);
}
#endif

struct SETTINGS {
  SETTINGS();
  void ApplyDefaults();

  SHADEREDITOR_OPTIONS shaderEditorOptions;
  RENDERER_SETTINGS rendererSettings;
  CAPTURE_SETTINGS captureSettings;

  std::map<int,std::string> midiRoutes;
  std::vector<std::pair<std::string,std::string> > texturePaths;

  int nDebugOutputHeight;
  int nTexPreviewWidth;
  float fFFTSmoothingFactor;
  float fFFTSlightSmoothingFactor;
};

SETTINGS::SETTINGS()
{
  ApplyDefaults();
}

void SETTINGS::ApplyDefaults()
{
  // Renderer settings
  rendererSettings.bVsync = false;
#ifdef _DEBUG
  rendererSettings.nWidth = 1280;
  rendererSettings.nHeight = 720;
  rendererSettings.windowMode = RENDERER_WINDOWMODE_WINDOWED;
#else
  rendererSettings.nWidth = 1920;
  rendererSettings.nHeight = 1080;
  rendererSettings.windowMode = RENDERER_WINDOWMODE_FULLSCREEN;
#endif

  // Shader editor options
  shaderEditorOptions.nFontSize = 16;
#ifdef _WIN32
  shaderEditorOptions.sFontPath = "c:\\Windows\\Fonts\\cour.ttf";
#elif __APPLE__
  shaderEditorOptions.sFontPath = "/Library/Fonts/Courier New.ttf";
#else
  shaderEditorOptions.sFontPath = "/usr/share/fonts/corefonts/cour.ttf";
#endif
  shaderEditorOptions.nOpacity = 0xC0;
  shaderEditorOptions.bUseSpacesForTabs = true;
  shaderEditorOptions.nTabSize = 2;
  shaderEditorOptions.bVisibleWhitespace = false;

  // Capture settings
  captureSettings.bNDIEnabled = true;
  captureSettings.sNDIConnectionString = "";
  captureSettings.sNDIIdentifier = "";
  captureSettings.fNDIFrameRate = 60.0;
  captureSettings.bNDIProgressive = true;

  // Other settings
  nDebugOutputHeight = 200;
  nTexPreviewWidth = 64;
  fFFTSmoothingFactor = 0.9f; // higher value, smoother FFT
  fFFTSlightSmoothingFactor = 0.6f; // higher value, smoother FFT
}

void LoadConfiguration(SETTINGS * settings)
{
  char szConfig[65535];
  FILE * fConf = fopen("config.json","rb");

  if (!fConf)
    return;

  printf("Config file found, parsing...\n");

  memset( szConfig, 0, 65535 );
  int n = fread( szConfig, 1, 65535, fConf );
  fclose(fConf);

  jsonxx::Object o;
  o.parse( szConfig );

  if (o.has<jsonxx::Object>("rendering"))
  {
    if (o.get<jsonxx::Object>("rendering").has<jsonxx::Number>("fftSmoothFactor"))
      settings->fFFTSmoothingFactor = o.get<jsonxx::Object>("rendering").get<jsonxx::Number>("fftSmoothFactor");
  }
  if (o.has<jsonxx::Object>("textures"))
  {
    std::map<std::string, jsonxx::Value*> tex = o.get<jsonxx::Object>("textures").kv_map();
    for (std::map<std::string, jsonxx::Value*>::iterator it = tex.begin(); it != tex.end(); it++)
    {
      const std::string& name = it->first;
      const std::string path = it->second->string_value_->c_str();
      settings->texturePaths.push_back( std::make_pair( name, path ) );
    }
  }
  if (o.has<jsonxx::Object>("font"))
  {
    SHADEREDITOR_OPTIONS& options = settings->shaderEditorOptions;

    if (o.get<jsonxx::Object>("font").has<jsonxx::Number>("size"))
      options.nFontSize = o.get<jsonxx::Object>("font").get<jsonxx::Number>("size");
    if (o.get<jsonxx::Object>("font").has<jsonxx::String>("file"))
      options.sFontPath = o.get<jsonxx::Object>("font").get<jsonxx::String>("file");
  }
  if (o.has<jsonxx::Object>("gui"))
  {
    SHADEREDITOR_OPTIONS& options = settings->shaderEditorOptions;

    if (o.get<jsonxx::Object>("gui").has<jsonxx::Number>("outputHeight"))
      settings->nDebugOutputHeight = o.get<jsonxx::Object>("gui").get<jsonxx::Number>("outputHeight");
    if (o.get<jsonxx::Object>("gui").has<jsonxx::Number>("texturePreviewWidth"))
      settings->nTexPreviewWidth = o.get<jsonxx::Object>("gui").get<jsonxx::Number>("texturePreviewWidth");
    if (o.get<jsonxx::Object>("gui").has<jsonxx::Number>("opacity"))
      options.nOpacity = o.get<jsonxx::Object>("gui").get<jsonxx::Number>("opacity");
    if (o.get<jsonxx::Object>("gui").has<jsonxx::Boolean>("spacesForTabs"))
      options.bUseSpacesForTabs = o.get<jsonxx::Object>("gui").get<jsonxx::Boolean>("spacesForTabs");
    if (o.get<jsonxx::Object>("gui").has<jsonxx::Number>("tabSize"))
      options.nTabSize = o.get<jsonxx::Object>("gui").get<jsonxx::Number>("tabSize");
    if (o.get<jsonxx::Object>("gui").has<jsonxx::Boolean>("visibleWhitespace"))
      options.bVisibleWhitespace = o.get<jsonxx::Object>("gui").get<jsonxx::Boolean>("visibleWhitespace");
  }
  if (o.has<jsonxx::Object>("midi"))
  {
    std::map<std::string, jsonxx::Value*> tex = o.get<jsonxx::Object>("midi").kv_map();
    for (std::map<std::string, jsonxx::Value*>::iterator it = tex.begin(); it != tex.end(); it++)
    {
      settings->midiRoutes.insert( std::make_pair( it->second->number_value_, it->first ) );
    }
  }
  if (o.has<jsonxx::Object>("ndi"))
  {
    CAPTURE_SETTINGS& captureSettings = settings->captureSettings;

    if (o.get<jsonxx::Object>("ndi").has<jsonxx::Boolean>("enabled"))
      captureSettings.bNDIEnabled = o.get<jsonxx::Object>("ndi").get<jsonxx::Boolean>("enabled");
    if (o.get<jsonxx::Object>("ndi").has<jsonxx::String>("connectionString"))
      captureSettings.sNDIConnectionString = o.get<jsonxx::Object>("ndi").get<jsonxx::String>("connectionString");
    if (o.get<jsonxx::Object>("ndi").has<jsonxx::String>("identifier"))
      captureSettings.sNDIIdentifier = o.get<jsonxx::Object>("ndi").get<jsonxx::String>("identifier");
    if (o.get<jsonxx::Object>("ndi").has<jsonxx::Number>("frameRate"))
      captureSettings.fNDIFrameRate = o.get<jsonxx::Object>("ndi").get<jsonxx::Number>("frameRate");
    if (o.get<jsonxx::Object>("ndi").has<jsonxx::Boolean>("progressive"))
      captureSettings.bNDIProgressive = o.get<jsonxx::Object>("ndi").get<jsonxx::Boolean>("progressive");
  }
}

int main()
{
#ifdef __APPLE__
  changeToAppsCurrentDirectory();
#endif

  SETTINGS settings;
  RENDERER_SETTINGS& rendererSettings = settings.rendererSettings;

#ifndef _DEBUG
  if (!Renderer::OpenSetupDialog( &rendererSettings ))
    return -1;
#endif

  if (!Renderer::Open( rendererSettings ))
  {
    printf("Renderer::Open failed\n");
    return -1;
  }

  if (!FFT::Open())
  {
    printf("FFT::Open() failed, continuing anyway...\n");
    //return -1;
  }

  if (!MIDI::Open())
  {
    printf("MIDI::Open() failed, continuing anyway...\n");
    //return -1;
  }

  LoadConfiguration( &settings );

  const CAPTURE_SETTINGS& captureSettings = settings.captureSettings;
  Capture::ApplySettings( captureSettings );

  if ( !Capture::Open( rendererSettings ) )
  {
    printf("Initializing capture system failed!\n");
    return 0;
  }

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );
  Renderer::Texture * texFFTSmoothed = Renderer::Create1DR32Texture( FFT_SIZE );
  Renderer::Texture * texFFTIntegrated = Renderer::Create1DR32Texture( FFT_SIZE );

  printf("Loading textures...\n");

  std::map<std::string,Renderer::Texture*> textures;
  std::vector<std::pair<std::string,std::string> >::const_iterator it;
  for (it = settings.texturePaths.begin(); it != settings.texturePaths.end(); ++it)
  {
      const std::string& name = it->first;
      const std::string& path = it->second;

      const char * fn = path.c_str();
      printf("* %s...\n",fn);
      Renderer::Texture * tex = Renderer::CreateRGBA8TextureFromFile( fn );
      if (!tex)
      {
        printf("Renderer::CreateRGBA8TextureFromFile(%s) failed\n",fn);
        return -1;
      }
      textures.insert( std::make_pair( it->first, tex ) );
  }

  bool shaderInitSuccessful = false;
  char szShader[65535];
  char szError[4096];
  FILE * f = fopen(Renderer::defaultShaderFilename,"rb");
  if (f)
  {
    printf("Loading last shader...\n");

    memset( szShader, 0, 65535 );
    int n = fread( szShader, 1, 65535, f );
    fclose(f);
    if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
    {
      printf("Last shader works fine.\n");
      shaderInitSuccessful = true;
    }
  }

  const std::map<int,std::string>& midiRoutes = settings.midiRoutes;

  if (!shaderInitSuccessful)
  {
    printf("No valid last shader found, falling back to default...\n");

    std::string sDefShader = Renderer::defaultShader;

    std::vector<std::string> tokens;
    for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
      tokens.push_back(it->first);
    ReplaceTokens(sDefShader, "{%textures:begin%}", "{%textures:name%}", "{%textures:end%}", tokens);

    tokens.clear();
    for (std::map<int,std::string>::const_iterator it = midiRoutes.begin(); it != midiRoutes.end(); it++)
      tokens.push_back(it->second);
    ReplaceTokens(sDefShader, "{%midi:begin%}", "{%midi:name%}", "{%midi:end%}", tokens);

    strncpy( szShader, sDefShader.c_str(), 65535 );
    if (!Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
    {
      printf("Default shader compile failed:\n");
      puts(szError);
      assert(0);
    }
  }

  Misc::InitKeymaps();

#ifdef SCI_LEXER
  Scintilla_LinkLexers();
#endif
  Scintilla::Surface * surface = Scintilla::Surface::Allocate( SC_TECHNOLOGY_DEFAULT );
  surface->Init( NULL );

  int nMargin = 20;

  bool bTexPreviewVisible = true;
  const int nDebugOutputHeight = settings.nDebugOutputHeight;
  const int nTexPreviewWidth = settings.nTexPreviewWidth;

  SHADEREDITOR_OPTIONS& options = settings.shaderEditorOptions;
  options.rect = Scintilla::PRectangle( nMargin, nMargin, rendererSettings.nWidth - nMargin - nTexPreviewWidth - nMargin, rendererSettings.nHeight - nMargin * 2 - nDebugOutputHeight );
  ShaderEditor mShaderEditor( surface );
  mShaderEditor.Initialise( options );
  mShaderEditor.SetText( szShader );

  options.rect = Scintilla::PRectangle( nMargin, rendererSettings.nHeight - nMargin - nDebugOutputHeight, rendererSettings.nWidth - nMargin - nTexPreviewWidth - nMargin, rendererSettings.nHeight - nMargin );
  ShaderEditor mDebugOutput( surface );
  mDebugOutput.Initialise( options );
  mDebugOutput.SetText( "" );
  mDebugOutput.SetReadOnly(true);

  static float fftData[FFT_SIZE];
  memset(fftData, 0, sizeof(float) * FFT_SIZE);
  static float fftDataSmoothed[FFT_SIZE];
  memset(fftDataSmoothed, 0, sizeof(float) * FFT_SIZE);


  static float fftDataSlightlySmoothed[FFT_SIZE];
  memset(fftDataSlightlySmoothed, 0, sizeof(float) * FFT_SIZE);
  static float fftDataIntegrated[FFT_SIZE];
  memset(fftDataIntegrated, 0, sizeof(float) * FFT_SIZE);

  bool bShowGui = true;
  Timer::Start();
  float fNextTick = 0.1;
  while (!Renderer::WantsToQuit())
  {
    float time = Timer::GetTime() / 1000.0; // seconds
    Renderer::StartFrame();

    for(int i=0; i<Renderer::mouseEventBufferCount; i++)
    {
      if (bShowGui)
      {
        switch (Renderer::mouseEventBuffer[i].eventType)
        {
          case Renderer::MOUSEEVENTTYPE_MOVE:
            mShaderEditor.ButtonMovePublic( Scintilla::Point( Renderer::mouseEventBuffer[i].x, Renderer::mouseEventBuffer[i].y ) );
            break;
          case Renderer::MOUSEEVENTTYPE_DOWN:
            mShaderEditor.ButtonDown( Scintilla::Point( Renderer::mouseEventBuffer[i].x, Renderer::mouseEventBuffer[i].y ), time * 1000, false, false, false );
            break;
          case Renderer::MOUSEEVENTTYPE_UP:
            mShaderEditor.ButtonUp( Scintilla::Point( Renderer::mouseEventBuffer[i].x, Renderer::mouseEventBuffer[i].y ), time * 1000, false );
            break;
          case Renderer::MOUSEEVENTTYPE_SCROLL:
            mShaderEditor.WndProc( SCI_LINESCROLL, -Renderer::mouseEventBuffer[i].x, -Renderer::mouseEventBuffer[i].y);
            break;
        }
      }
    }
    Renderer::mouseEventBufferCount = 0;

    for(int i=0; i<Renderer::keyEventBufferCount; i++)
    {
      if (Renderer::keyEventBuffer[i].scanCode == 283) // F2
      {
        if (bTexPreviewVisible)
        {
          mShaderEditor.SetPosition( Scintilla::PRectangle( nMargin, nMargin, rendererSettings.nWidth - nMargin, rendererSettings.nHeight - nMargin * 2 - nDebugOutputHeight ) );
          mDebugOutput .SetPosition( Scintilla::PRectangle( nMargin, rendererSettings.nHeight - nMargin - nDebugOutputHeight, rendererSettings.nWidth - nMargin, rendererSettings.nHeight - nMargin ) );
          bTexPreviewVisible = false;
        }
        else
        {
          mShaderEditor.SetPosition( Scintilla::PRectangle( nMargin, nMargin, rendererSettings.nWidth - nMargin - nTexPreviewWidth - nMargin, rendererSettings.nHeight - nMargin * 2 - nDebugOutputHeight ) );
          mDebugOutput .SetPosition( Scintilla::PRectangle( nMargin, rendererSettings.nHeight - nMargin - nDebugOutputHeight, rendererSettings.nWidth - nMargin - nTexPreviewWidth - nMargin, rendererSettings.nHeight - nMargin ) );
          bTexPreviewVisible = true;
        }
      }
      else if (Renderer::keyEventBuffer[i].scanCode == 286 || (Renderer::keyEventBuffer[i].ctrl && Renderer::keyEventBuffer[i].scanCode == 'r')) // F5
      {
        mShaderEditor.GetText(szShader,65535);
        if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
        {
          FILE * f = fopen(Renderer::defaultShaderFilename,"wb");
          fwrite( szShader, strlen(szShader), 1, f );
          fclose(f);
          mDebugOutput.SetText( "" );
        }
        else
        {
          mDebugOutput.SetText( szError );
        }
      }
      else if (Renderer::keyEventBuffer[i].scanCode == 292) // F11
      {
        bShowGui = !bShowGui;
      }
      else if (bShowGui)
      {
        bool consumed = false;
        if (Renderer::keyEventBuffer[i].scanCode)
        {
        mShaderEditor.KeyDown(
          iswalpha(Renderer::keyEventBuffer[i].scanCode) ? towupper(Renderer::keyEventBuffer[i].scanCode) : Renderer::keyEventBuffer[i].scanCode,
          Renderer::keyEventBuffer[i].shift,
          Renderer::keyEventBuffer[i].ctrl,
          Renderer::keyEventBuffer[i].alt,
          &consumed);
        }
        if (!consumed && Renderer::keyEventBuffer[i].character)
        {
          char    utf8[5] = {0,0,0,0,0};
          wchar_t utf16[2] = {Renderer::keyEventBuffer[i].character, 0};
          Scintilla::UTF8FromUTF16(utf16, 1, utf8, 4 * sizeof(char));
          mShaderEditor.AddCharUTF(utf8, strlen(utf8));
        }

      }
    }
    Renderer::keyEventBufferCount = 0;

    Renderer::SetShaderConstant( "fGlobalTime", time );
    Renderer::SetShaderConstant( "v2Resolution", rendererSettings.nWidth, rendererSettings.nHeight );

    for (std::map<int,std::string>::const_iterator it = midiRoutes.begin(); it != midiRoutes.end(); it++)
    {
      Renderer::SetShaderConstant( it->second.c_str(), MIDI::GetCCValue( it->first ) );
    }


    if (FFT::GetFFT(fftData))
    {
      Renderer::UpdateR32Texture( texFFT, fftData );

      const static float maxIntegralValue = 1024.0f;
      const float fFFTSmoothingFactor = settings.fFFTSmoothingFactor;
      const float fFFTSlightSmoothingFactor = settings.fFFTSlightSmoothingFactor;
      for ( int i = 0; i < FFT_SIZE; i++ )
      {
        fftDataSmoothed[i] = fftDataSmoothed[i] * fFFTSmoothingFactor + (1 - fFFTSmoothingFactor) * fftData[i];

        fftDataSlightlySmoothed[i] = fftDataSlightlySmoothed[i] * fFFTSlightSmoothingFactor + (1 - fFFTSlightSmoothingFactor) * fftData[i];
        fftDataIntegrated[i] = fftDataIntegrated[i] + fftDataSlightlySmoothed[i];
        if (fftDataIntegrated[i] > maxIntegralValue) {
          fftDataIntegrated[i] -= maxIntegralValue;
        }
      }

      Renderer::UpdateR32Texture( texFFTSmoothed, fftDataSmoothed );
      Renderer::UpdateR32Texture( texFFTIntegrated, fftDataIntegrated );
    }

    Renderer::SetShaderTexture( "texFFT", texFFT );
    Renderer::SetShaderTexture( "texFFTSmoothed", texFFTSmoothed );
    Renderer::SetShaderTexture( "texFFTIntegrated", texFFTIntegrated );

    for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
    {
      Renderer::SetShaderTexture( (char*)it->first.c_str(), it->second );
    }

    Renderer::RenderFullscreenQuad();

    Renderer::StartTextRendering();

    if (bShowGui)
    {
      if (time > fNextTick)
      {
        mShaderEditor.Tick();
        mDebugOutput.Tick();
        fNextTick = time + 0.1;
      }

      mShaderEditor.Paint();
      mDebugOutput.Paint();

      Renderer::SetTextRenderingViewport( Scintilla::PRectangle(0,0,Renderer::nWidth,Renderer::nHeight) );

      if (bTexPreviewVisible)
      {
        int y1 = nMargin;
        int x1 = rendererSettings.nWidth - nMargin - nTexPreviewWidth;
        int x2 = rendererSettings.nWidth - nMargin;
        for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
        {
          int y2 = y1 + nTexPreviewWidth * (it->second->height / (float)it->second->width);
          Renderer::BindTexture( it->second );
          Renderer::RenderQuad(
            Renderer::Vertex( x1, y1, 0xccFFFFFF, 0.0, 0.0 ),
            Renderer::Vertex( x2, y1, 0xccFFFFFF, 1.0, 0.0 ),
            Renderer::Vertex( x2, y2, 0xccFFFFFF, 1.0, 1.0 ),
            Renderer::Vertex( x1, y2, 0xccFFFFFF, 0.0, 1.0 )
          );
          surface->DrawTextNoClip( Scintilla::PRectangle(x1,y1,x2,y2), *mShaderEditor.GetTextFont(), y2 - 5.0, it->first.c_str(), it->first.length(), 0xffFFFFFF, 0x00000000);
          y1 = y2 + nMargin;
        }
      }

      char szLayout[255];
      Misc::GetKeymapName(szLayout);
      std::string sHelp = "F2 - toggle texture preview   F5 or Ctrl-R - recompile shader   F11 - hide GUI   Current keymap: ";
      sHelp += szLayout;
      surface->DrawTextNoClip( Scintilla::PRectangle(20,Renderer::nHeight - 20,100,Renderer::nHeight), *mShaderEditor.GetTextFont(), Renderer::nHeight - 5.0, sHelp.c_str(), sHelp.length(), 0x80FFFFFF, 0x00000000);
    }


    Renderer::EndTextRendering();

    Renderer::EndFrame();

    Capture::CaptureFrame();
  }


  delete surface;

  MIDI::Close();
  FFT::Close();

  Renderer::ReleaseTexture( texFFT );
  Renderer::ReleaseTexture( texFFTSmoothed );
  for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
  {
    Renderer::ReleaseTexture( it->second );
  }

  Renderer::Close();
  return 0;
}
