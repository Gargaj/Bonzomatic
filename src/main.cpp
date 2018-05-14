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
#else
#include <unistd.h>
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

int main(int argc, char *argv[])
{
#ifdef __APPLE__
  changeToAppsCurrentDirectory();
#endif

  jsonxx::Object options;
  FILE * fConf = fopen( (argc > 1) ? argv[1] : "config.json","rb");
  if (fConf)
  {
    printf("Config file found, parsing...\n");

    char szConfig[65535];
    memset( szConfig, 0, 65535 );
    int n = fread( szConfig, 1, 65535, fConf );
    fclose(fConf);

    options.parse( szConfig );
  }

  RENDERER_SETTINGS settings;
  settings.bVsync = false;
#ifdef _DEBUG
  settings.nWidth = 1280;
  settings.nHeight = 720;
  settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;
#else
  settings.nWidth = 1920;
  settings.nHeight = 1080;
  settings.windowMode = RENDERER_WINDOWMODE_FULLSCREEN;
  if (options.has<jsonxx::Object>("window"))
  {
    if (options.get<jsonxx::Object>("window").has<jsonxx::Number>("width"))
      settings.nWidth = options.get<jsonxx::Object>("window").get<jsonxx::Number>("width");
    if (options.get<jsonxx::Object>("window").has<jsonxx::Number>("height"))
      settings.nHeight = options.get<jsonxx::Object>("window").get<jsonxx::Number>("height");
    if (options.get<jsonxx::Object>("window").has<jsonxx::Boolean>("fullscreen"))
      settings.windowMode = options.get<jsonxx::Object>("window").get<jsonxx::Boolean>("fullscreen") ? RENDERER_WINDOWMODE_FULLSCREEN : RENDERER_WINDOWMODE_WINDOWED;
  }
  if (!Renderer::OpenSetupDialog( &settings ))
    return -1;
#endif

  if (!Renderer::Open( &settings ))
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

  std::map<std::string,Renderer::Texture*> textures;
  std::map<int,std::string> midiRoutes;

  SHADEREDITOR_OPTIONS editorOptions;
  editorOptions.nFontSize = 16;
#ifdef _WIN32
  editorOptions.sFontPath = "c:\\Windows\\Fonts\\cour.ttf";
#elif __APPLE__
  editorOptions.sFontPath = "/Library/Fonts/Courier New.ttf";
#else
  // Linux case
  // TODO: use fonts.conf(5) or X resources or something like that
  const char* fontPaths[] = {
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
    "/usr/share/fonts/TTF/FreeMono.ttf",
    "/usr/share/fonts/TTF/LiberationMono-Regular.ttf",
    "/usr/share/fonts/TTF/VeraMono.ttf",
    "/usr/share/fonts/corefonts/cour.ttf",
    "/usr/share/fonts/truetype/msttcorefonts/cour.ttf",
    NULL
  };
  editorOptions.sFontPath = "";
  for (int i = 0; fontPaths[i]; ++i)
  {
    if (access(fontPaths[i], R_OK) != -1)
    {
      editorOptions.sFontPath = fontPaths[i];
      break;
    }
  }
  // aiee - no font found, but don't report yet, it might still get changed
  // though config.json
#endif
  editorOptions.nOpacity = 0xC0;
  editorOptions.bUseSpacesForTabs = true;
  editorOptions.nTabSize = 2;
  editorOptions.bVisibleWhitespace = false;

  int nDebugOutputHeight = 200;
  int nTexPreviewWidth = 64;
  float fFFTSmoothingFactor = 0.9f; // higher value, smoother FFT
  float fFFTSlightSmoothingFactor = 0.6f; // higher value, smoother FFT

  std::string sPostExitCmd;

  if (!options.empty())
  {
    if (options.has<jsonxx::Object>("rendering"))
    {
      if (options.get<jsonxx::Object>("rendering").has<jsonxx::Number>("fftSmoothFactor"))
        fFFTSmoothingFactor = options.get<jsonxx::Object>("rendering").get<jsonxx::Number>("fftSmoothFactor");
    }

    if (options.has<jsonxx::Object>("textures"))
    {
      printf("Loading textures...\n");
      std::map<std::string, jsonxx::Value*> tex = options.get<jsonxx::Object>("textures").kv_map();
      for (std::map<std::string, jsonxx::Value*>::iterator it = tex.begin(); it != tex.end(); it++)
      {
        char * fn = (char*)it->second->string_value_->c_str();
        printf("* %s...\n",fn);
        Renderer::Texture * tex = Renderer::CreateRGBA8TextureFromFile( fn );
        if (!tex)
        {
          printf("Renderer::CreateRGBA8TextureFromFile(%s) failed\n",fn);
          return -1;
        }
        textures.insert( std::make_pair( it->first, tex ) );
      }
    }
    if (options.has<jsonxx::Object>("font"))
    {
      if (options.get<jsonxx::Object>("font").has<jsonxx::Number>("size"))
        editorOptions.nFontSize = options.get<jsonxx::Object>("font").get<jsonxx::Number>("size");
      if (options.get<jsonxx::Object>("font").has<jsonxx::String>("file"))
      {
        std::string fontpath = options.get<jsonxx::Object>("font").get<jsonxx::String>("file");
        // TODO: port this to other platforms
#if !defined(_WIN32) && !defined(__APPLE__)
        if (access(fontpath.c_str(), R_OK) != -1)
          editorOptions.sFontPath = fontpath;
        else
        {
          printf("Couldn't open the font file '%s'.\n", fontpath.c_str());
          return -1;
        }
#else
        editorOptions.sFontPath = fontpath;
#endif
      }
      else if (!editorOptions.sFontPath.size()) // coudn't find a default font
      {
        printf("Couldn't find any of the default fonts. Please specify one in config.json\n");
        return -1;
      }
    }
    if (options.has<jsonxx::Object>("gui"))
    {
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Number>("outputHeight"))
        nDebugOutputHeight = options.get<jsonxx::Object>("gui").get<jsonxx::Number>("outputHeight");
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Number>("texturePreviewWidth"))
        nTexPreviewWidth = options.get<jsonxx::Object>("gui").get<jsonxx::Number>("texturePreviewWidth");
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Number>("opacity"))
        editorOptions.nOpacity = options.get<jsonxx::Object>("gui").get<jsonxx::Number>("opacity");
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Boolean>("spacesForTabs"))
        editorOptions.bUseSpacesForTabs = options.get<jsonxx::Object>("gui").get<jsonxx::Boolean>("spacesForTabs");
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Number>("tabSize"))
        editorOptions.nTabSize = options.get<jsonxx::Object>("gui").get<jsonxx::Number>("tabSize");
      if (options.get<jsonxx::Object>("gui").has<jsonxx::Boolean>("visibleWhitespace"))
        editorOptions.bVisibleWhitespace = options.get<jsonxx::Object>("gui").get<jsonxx::Boolean>("visibleWhitespace");
    }
    if (options.has<jsonxx::Object>("midi"))
    {
      std::map<std::string, jsonxx::Value*> tex = options.get<jsonxx::Object>("midi").kv_map();
      for (std::map<std::string, jsonxx::Value*>::iterator it = tex.begin(); it != tex.end(); it++)
      {
        midiRoutes.insert( std::make_pair( it->second->number_value_, it->first ) );
      }
    }
    if (options.has<jsonxx::String>("postExitCmd"))
    {
      sPostExitCmd = options.get<jsonxx::String>("postExitCmd");
    }
    Capture::LoadSettings( options );
  }
  else if (!editorOptions.sFontPath.size())
  {
    printf("Couldn't find any of the default fonts. Please specify one in config.json\n");
    return -1;
  }
  if (!Capture::Open(settings))
  {
    printf("Initializing capture system failed!\n");
    return 0;
  }

  Renderer::Texture * texFFT = Renderer::Create1DR32Texture( FFT_SIZE );
  Renderer::Texture * texFFTSmoothed = Renderer::Create1DR32Texture( FFT_SIZE );
  Renderer::Texture * texFFTIntegrated = Renderer::Create1DR32Texture( FFT_SIZE );

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
    else {
      printf("Shader error:\n%s\n", szError);
    }
  }
  if (!shaderInitSuccessful)
  {
    printf("No valid last shader found, falling back to default...\n");

    std::string sDefShader = Renderer::defaultShader;

    std::vector<std::string> tokens;
    for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
      tokens.push_back(it->first);
    ReplaceTokens(sDefShader, "{%textures:begin%}", "{%textures:name%}", "{%textures:end%}", tokens);

    tokens.clear();
    for (std::map<int,std::string>::iterator it = midiRoutes.begin(); it != midiRoutes.end(); it++)
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

  editorOptions.rect = Scintilla::PRectangle( nMargin, nMargin, settings.nWidth - nMargin - nTexPreviewWidth - nMargin, settings.nHeight - nMargin * 2 - nDebugOutputHeight );
  ShaderEditor mShaderEditor( surface );
  mShaderEditor.Initialise( editorOptions );
  mShaderEditor.SetText( szShader );

  editorOptions.rect = Scintilla::PRectangle( nMargin, settings.nHeight - nMargin - nDebugOutputHeight, settings.nWidth - nMargin - nTexPreviewWidth - nMargin, settings.nHeight - nMargin );
  ShaderEditor mDebugOutput( surface );
  mDebugOutput.Initialise( editorOptions );
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
    bool newShader = false;
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
          mShaderEditor.SetPosition( Scintilla::PRectangle( nMargin, nMargin, settings.nWidth - nMargin, settings.nHeight - nMargin * 2 - nDebugOutputHeight ) );
          mDebugOutput .SetPosition( Scintilla::PRectangle( nMargin, settings.nHeight - nMargin - nDebugOutputHeight, settings.nWidth - nMargin, settings.nHeight - nMargin ) );
          bTexPreviewVisible = false;
        }
        else
        {
          mShaderEditor.SetPosition( Scintilla::PRectangle( nMargin, nMargin, settings.nWidth - nMargin - nTexPreviewWidth - nMargin, settings.nHeight - nMargin * 2 - nDebugOutputHeight ) );
          mDebugOutput .SetPosition( Scintilla::PRectangle( nMargin, settings.nHeight - nMargin - nDebugOutputHeight, settings.nWidth - nMargin - nTexPreviewWidth - nMargin, settings.nHeight - nMargin ) );
          bTexPreviewVisible = true;
        }
      }
      else if (Renderer::keyEventBuffer[i].scanCode == 286 || (Renderer::keyEventBuffer[i].ctrl && Renderer::keyEventBuffer[i].scanCode == 'r')) // F5
      {
        mShaderEditor.GetText(szShader,65535);
        if (Renderer::ReloadShader( szShader, strlen(szShader), szError, 4096 ))
        {
          // Shader compilation successful; we set a flag to save if the frame render was successful
          // (If there is a driver crash, don't save.)
          newShader = true;
        }
        else
        {
          mDebugOutput.SetText( szError );
        }
      }
      else if ((Renderer::keyEventBuffer[i].alt && Renderer::keyEventBuffer[i].scanCode == 'r')) // F5
      {

        FILE * f = fopen(Renderer::defaultShaderFilename, "rb");
        if (f)
        {
          memset(szShader, 0, 65535);
          int n = fread(szShader, 1, 65535, f);
          fclose(f);
          if (Renderer::ReloadShader(szShader, strlen(szShader), szError, 4096))
          {
            shaderInitSuccessful = true;
            mShaderEditor.SetText(szShader);
            // Shader compilation successful; we set a flag to save if the frame render was successful
            // (If there is a driver crash, don't save.)
            //newShader = true;
          }
          else {
            printf("Shader error:\n%s\n", szError);
            mDebugOutput.SetText(szError);
          }
        }
        else
        {
          mDebugOutput.SetText(szError);
        }
      }
      else if (Renderer::keyEventBuffer[i].scanCode == 292 || (Renderer::keyEventBuffer[i].ctrl && Renderer::keyEventBuffer[i].scanCode == 'f')) // F11 or Ctrl/Cmd-f  
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
    Renderer::SetShaderConstant( "v2Resolution", settings.nWidth, settings.nHeight );

    for (std::map<int,std::string>::iterator it = midiRoutes.begin(); it != midiRoutes.end(); it++)
    {
      Renderer::SetShaderConstant( (char*)it->second.c_str(), MIDI::GetCCValue( it->first ) );
    }


    if (FFT::GetFFT(fftData))
    {
      Renderer::UpdateR32Texture( texFFT, fftData );

      const static float maxIntegralValue = 1024.0f;
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
        int x1 = settings.nWidth - nMargin - nTexPreviewWidth;
        int x2 = settings.nWidth - nMargin;
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

    if (newShader)
    {
      // Frame render successful, save shader
      FILE * f = fopen(Renderer::defaultShaderFilename,"wb");
      if (f)
      {
        fwrite( szShader, strlen(szShader), 1, f );
        fclose(f);
        mDebugOutput.SetText( "" );
      }
      else
      {
        mDebugOutput.SetText( "Unable to save shader! Your work will be lost when you quit!" );
      }
    }
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

  if ( !sPostExitCmd.empty() )
  {
    Misc::ExecuteCommand( (char*)sPostExitCmd.c_str(), Renderer::defaultShaderFilename );
  }

  return 0;
}
