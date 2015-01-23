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

  bool Open( RENDERER_SETTINGS * settings );
  
  void StartFrame();
  void EndFrame();
  bool WantsToQuit();

  void RenderFullscreenQuad();

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize );
  void SetShaderConstant( char * szConstName, float x );
  void SetShaderConstant( char * szConstName, float x, float y );

  void SwitchToTextRenderingMode();

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
}