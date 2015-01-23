#include <Platform.h>

typedef enum {
  RENDERER_WINDOWMODE_WINDOWED = 0,
  RENDERER_WINDOWMODE_FULLSCREEN,
  RENDERER_WINDOWMODE_BORDERLESS
} RENDERER_WINDOWMODE;

typedef struct 
{
  int nWidth;
  int nHeight;
  RENDERER_WINDOWMODE windowMode;
} RENDERER_SETTINGS;

namespace Renderer
{
  extern char defaultShader[65536];

  extern int nWidth;
  extern int nHeight;

  bool OpenSetupDialog( RENDERER_SETTINGS * settings );
  bool Open( RENDERER_SETTINGS * settings );
  
  void StartFrame();
  void EndFrame();
  bool WantsToQuit();

  void RenderFullscreenQuad();

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize );
  void SetShaderConstant( char * szConstName, float x );
  void SetShaderConstant( char * szConstName, float x, float y );

  void StartTextRendering();
  void SetTextRenderingViewport( Scintilla::PRectangle rect );
  void EndTextRendering();

  void Close();

  enum TEXTURETYPE
  {
    TEXTURETYPE_1D = 1,
    TEXTURETYPE_2D = 2,
  };

  struct Texture
  {
    int width;
    int height;
    TEXTURETYPE type;
  };

  Texture * CreateRGBA8TextureFromFile( char * szFilename );
  Texture * Create1DR32Texture( int w );
  bool UpdateR32Texture( Texture * tex, float * data );
  void SetShaderTexture( char * szTextureName, Texture * tex );

  struct KeyEvent
  {
    int character;
    int scanCode;
    bool ctrl;
    bool shift;
    bool alt;
  };
  extern KeyEvent keyEventBuffer[512];
  extern int keyEventBufferCount;

  enum MOUSEBUTTON
  {
    MOUSEBUTTON_LEFT = 0,
    MOUSEBUTTON_RIGHT,
    MOUSEBUTTON_MIDDLE,
  };
  struct MouseEvent
  {
    int x;
    int y;
    MOUSEBUTTON button;
  };
  extern MouseEvent mouseEventBuffer[512];
  extern int mouseEventBufferCount;
}