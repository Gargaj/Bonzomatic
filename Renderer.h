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
  bool Open( RENDERER_SETTINGS * settings );
  void StartFrame();
  void EndFrame();
  bool WantsToQuit();
  void Close();
}