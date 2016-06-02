// Some of this code was originally written for the ScintillaGL project by:
// Copyright 2011 by Mykhailo Parfeniuk

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <map>

#include "Platform.h"
#include "Scintilla.h"
#include "UniConversion.h"
#include "XPM.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "Renderer.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

//////////////////////////////////////////////////////////////////////////

// this is only used if we support loading external lexers, which we don't
DynamicLibrary *DynamicLibrary::Load(const char *modulePath)
{
  return NULL;
}

//////////////////////////////////////////////////////////////////////////

ColourDesired MakeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a=0xFF)
{
  return a<<24 | b<<16 | g<<8 | r;
}

ColourDesired Platform::Chrome()
{
  return MakeRGBA(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight()
{
  return MakeRGBA(0xff, 0xff, 0xff);
}

const char *Platform::DefaultFont()
{
  return "Lucida Console";
}

int Platform::DefaultFontSize()
{
  return 10;
}

unsigned int Platform::DoubleClickTime()
{
  return 500; 	// Half a second
}

bool Platform::MouseButtonBounce()
{
  return true;
}

void Platform::Assert(const char *c, const char *file, int line) 
{
  char buffer[2000];
  sprintf(buffer, "Assertion [%s] failed at %s %d", c, file, line);
  strcat(buffer, "\r\n");
  assert(false);
}
int Platform::Minimum(int a, int b) { return a<b ? a : b; }
int Platform::Maximum(int a, int b) { return a>b ? a : b; }
int Platform::Clamp(int val, int minVal, int maxVal) { return Minimum( maxVal, Maximum( val, minVal ) ); }

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...)
{
  char buffer[2000];
  va_list pArguments;
  va_start(pArguments, format);
  vsprintf(buffer,format,pArguments);
  va_end(pArguments);
  Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...)
{
}
#endif

//////////////////////////////////////////////////////////////////////////
// FONT

#define CHARACTER_COUNT 512 // first 512 chars of unicode should be sufficient

struct stbtt_Font
{
  stbtt_fontinfo fontinfo;
  stbtt_bakedchar cdata[CHARACTER_COUNT];
  float scale;
  Renderer::Texture * texture;
};

Font::Font() : fid(0)
{
}

Font::~Font()
{
}

void Font::Create(const FontParameters &fp)
{
  stbtt_Font* newFont = new stbtt_Font;

  FILE* f = fopen(fp.faceName, "rb");

  assert(f);

  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);

  int texSize = 512;
  unsigned char* buf = (unsigned char*)malloc(len);
  fread(buf, 1, len, f);
  fclose(f);

  unsigned char* bmp = new unsigned char[texSize*texSize];

  stbtt_BakeFontBitmap(buf, 0, fp.size, bmp, texSize, texSize, 0, CHARACTER_COUNT, newFont->cdata); // no guarantee this fits!

#ifdef _DEBUG
  FILE* dump = fopen("font.raw", "wb");
  fwrite(bmp,texSize,texSize,dump);
  fclose(dump);
#endif // _DEBUG

  newFont->texture = Renderer::CreateA8TextureFromData( texSize, texSize, bmp );

  stbtt_InitFont(&newFont->fontinfo, buf, 0);

  newFont->scale = stbtt_ScaleForPixelHeight(&newFont->fontinfo, fp.size);

  delete [] bmp;

  fid = newFont;
}

void Font::Release()
{
  if (fid)
  {
    free(((stbtt_Font*)fid)->fontinfo.data);
    Renderer::ReleaseTexture( ((stbtt_Font*)fid)->texture );
    delete (stbtt_Font*)fid;
  }
}

//////////////////////////////////////////////////////////////////////////
// SURFACE

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif
class SurfaceImpl : public Surface {
  ColourDesired penColour;
  float currentX;
  float currentY;
  bool unicodeMode;
  int codePage;
  bool initialised;
  PRectangle clipRect;
public:
  SurfaceImpl();
  virtual ~SurfaceImpl();

  void Init(WindowID wid);
  void Init(SurfaceID sid, WindowID wid);
  void InitPixMap(int width, int height, Surface *surface, WindowID wid);

  void Release();
  bool Initialised();
  void PenColour(ColourDesired fore);
  int LogPixelsY();
  int DeviceHeightFont(int points);
  void MoveTo(float x, float y);
  void LineTo(float x, float y);
  void Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back);
  void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back);
  void FillRectangle(PRectangle rc, ColourDesired back);
  void FillRectangle(PRectangle rc, Surface &surfacePattern);
  void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back);
  void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
    ColourDesired outline, int alphaOutline, int flags);
  void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back);
  void Copy(PRectangle rc, Point from, Surface &surfaceSource);

  //virtual void DrawPixmap(PRectangle rc, Point from, Pixmap pixmap);
  virtual void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage);

  void DrawTextBase(PRectangle rc, Font &font, float ybase, const char *s, int len, ColourDesired fore);
  void DrawTextNoClip(PRectangle rc, Font &font, float ybase, const char *s, int len, ColourDesired fore, ColourDesired back);
  void DrawTextClipped(PRectangle rc, Font &font, float ybase, const char *s, int len, ColourDesired fore, ColourDesired back);
  void DrawTextTransparent(PRectangle rc, Font &font, float ybase, const char *s, int len, ColourDesired fore);
  void MeasureWidths(Font &font, const char *s, int len, float *positions);
  float WidthText(Font &font, const char *s, int len);
  float WidthChar(Font &font, char ch);
  float Ascent(Font &font);
  float Descent(Font &font);
  float InternalLeading(Font &font);
  float ExternalLeading(Font &font);
  float Height(Font &font);
  float AverageCharWidth(Font &font);

  void MoveTo(int x, int y);
  void LineTo(int x, int y);

  void SetClip(PRectangle rc);
  void FlushCachedState();

  void SetUnicodeMode(bool unicodeMode_);
  void SetDBCSMode(int codePage);
};

#ifdef SCI_NAMESPACE
}
#endif

SurfaceImpl::SurfaceImpl() 
  : currentX(0), currentY(0)
{
  unicodeMode = false;
  codePage = 0;
  initialised = false;
}

SurfaceImpl::~SurfaceImpl()
{
}

void SurfaceImpl::Init( WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::Init( SurfaceID sid, WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::InitPixMap( int width, int height, Surface *surface, WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::Release() 
{
}

bool SurfaceImpl::Initialised()
{
  return initialised;
}

void SurfaceImpl::PenColour(ColourDesired fore) 
{
  penColour = fore;
}

int SurfaceImpl::LogPixelsY() 
{
  return 72;
}

int SurfaceImpl::DeviceHeightFont(int points) 
{
  int logPix = LogPixelsY();
  return (points * logPix + logPix / 2) / 72;
}

void SurfaceImpl::MoveTo(float x, float y) 
{
  currentX = x;
  currentY = y;
}

void SurfaceImpl::LineTo(float targetX, float targetY) 
{
  Renderer::RenderLine(
    Renderer::Vertex( currentX+0.5f, currentY+0.5f, penColour.AsLong() ),
    Renderer::Vertex(  targetX+0.5f,  targetY+0.5f, penColour.AsLong() )
  );
  currentX = targetX;
  currentY = targetY;
}

void SurfaceImpl::MoveTo( int x, int y )
{
  MoveTo( (float)x, (float)y );
}

void SurfaceImpl::LineTo( int x, int y )
{
  LineTo( (float)x, (float)y );
}

void SurfaceImpl::Polygon(Point* /*pts*/, int /*npts*/, ColourDesired /*fore*/, ColourDesired /*back*/) 
{
  assert(!"Implemented");
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) 
{
  FillRectangle(rc, back);
  PenColour(fore);
  MoveTo( rc.left , rc.top );
  LineTo( rc.right, rc.top );
  LineTo( rc.right, rc.bottom );
  LineTo( rc.left,  rc.bottom);
  LineTo( rc.left,  rc.top );
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourDesired back) 
{
  Renderer::BindTexture(NULL);
  
  Renderer::RenderQuad(
    Renderer::Vertex( rc.left , rc.top, back.AsLong() ),
    Renderer::Vertex( rc.right, rc.top, back.AsLong() ),
    Renderer::Vertex( rc.right, rc.bottom, back.AsLong() ),
    Renderer::Vertex( rc.left , rc.bottom, back.AsLong() )
    );
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface & surfacePattern) 
{
  FillRectangle( rc, 0xd0000000 ); 
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) 
{
  RectangleDraw( rc, fore, back ); 
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int /*cornerSize*/, ColourDesired fill, int alphaFill,
    ColourDesired /*outline*/, int /*alphaOutline*/, int /*flags*/) 
{
  unsigned int back = fill.AsLong() & 0xFFFFFF | ((alphaFill & 0xFF) << 24);
  FillRectangle(rc, back);
}

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
  assert(!"Implemented");
}

void SurfaceImpl::Ellipse(PRectangle /*rc*/, ColourDesired /*fore*/, ColourDesired /*back*/) 
{
  assert(!"Implemented");
}

void SurfaceImpl::Copy( PRectangle rc, Point from, Surface &surfaceSource )
{
  // we don't assert here because this is often used
  // however, we don't support it right now
}

void SurfaceImpl::DrawTextBase(PRectangle rc, Font &font, float ybase, const char *str, int len, ColourDesired fore) 
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();

  Renderer::BindTexture( realFont->texture );
  float x = rc.left, y = ybase;
  while (len-- > 0) 
  {
    unsigned int c = *str;
    unsigned int charLength = UTF8CharLength(c);
    if (charLength > 1)
    {
      c = 0;
      UTF16FromUTF8( str, charLength, (wchar_t*)&c, sizeof(unsigned int) );
    }
    if (c >= CHARACTER_COUNT)
      c = '?';
    stbtt_aligned_quad quad;
    stbtt_GetBakedQuad( realFont->cdata, realFont->texture->width, realFont->texture->height, c, &x, &y, &quad, 1 );
    
    Renderer::RenderQuad(
      Renderer::Vertex( quad.x0, quad.y0, fore.AsLong(), quad.s0, quad.t0 ),
      Renderer::Vertex( quad.x1, quad.y0, fore.AsLong(), quad.s1, quad.t0 ),
      Renderer::Vertex( quad.x1, quad.y1, fore.AsLong(), quad.s1, quad.t1 ),
      Renderer::Vertex( quad.x0, quad.y1, fore.AsLong(), quad.s0, quad.t1 )
    );
    str += charLength;
    len -= charLength - 1;
  }
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font, float ybase, const char *s, int len,
                                 ColourDesired fore, ColourDesired /*back*/)
{
  DrawTextBase(rc, font, ybase, s, len, fore);
}

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font, float ybase, const char *s, int len,
                                  ColourDesired fore, ColourDesired /*back*/)
{
  DrawTextBase(rc, font, ybase, s, len, fore);
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font, float ybase, const char *s, int len,
                                  ColourDesired fore)
{
  DrawTextBase(rc, font, ybase, s, len, fore);
}

void SurfaceImpl::MeasureWidths(Font & font, const char *str, int len, float *positions)
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  
  float position = 0;
  char * p = (char*)str;
  while (len-- > 0) 
  {
    unsigned int c = *p;
    unsigned int charLength = UTF8CharLength(c);
    if (charLength > 1)
    {
      c = 0;
      UTF16FromUTF8( str, charLength, (wchar_t*)&c, sizeof(unsigned int) );
    }
    if (c >= CHARACTER_COUNT)
      c = '?';

    int advance = 0, leftBearing = 0;
    
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    
    position     += advance;
    for (int i=0; i<charLength; i++) // we need to loop here because UTF8 characters count as multiple unless their position is the same
      *positions++  = position*realFont->scale;

    p += charLength;
    len -= charLength - 1;
  }
}

float SurfaceImpl::WidthText(Font & font, const char *str, int len)
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  
  float position = 0;
  while (len-- > 0) 
  {
    unsigned int c = *str;
    unsigned int charLength = UTF8CharLength(c);
    if (charLength > 1)
    {
      c = 0;
      UTF16FromUTF8( str, charLength, (wchar_t*)&c, sizeof(unsigned int) );
    }
    if (c >= CHARACTER_COUNT)
      c = '?';

    int advance = 0, leftBearing = 0;
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    position += advance*realFont->scale;//TODO: +Kerning

    str += charLength;
    len -= charLength - 1;
  }
  return position;
}

float SurfaceImpl::WidthChar(Font &font, char ch)
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  int advance, leftBearing;
  stbtt_GetCodepointHMetrics(&realFont->fontinfo, ch, &advance, &leftBearing);
  return advance * realFont->scale;
}

float SurfaceImpl::Ascent(Font &font)
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return ascent * realFont->scale;
}

float SurfaceImpl::Descent(Font &font)
{
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return -descent * realFont->scale;
}

float SurfaceImpl::InternalLeading(Font &)
{
  //WTF is this?????
  return 0;
}

float SurfaceImpl::ExternalLeading(Font& font)
{
  //WTF is this?????
  stbtt_Font* realFont = (stbtt_Font*)font.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return lineGap * realFont->scale;
}

float SurfaceImpl::Height(Font &font)
{
  return Ascent(font) + Descent(font);
}

float SurfaceImpl::AverageCharWidth(Font &font) 
{
  return WidthChar(font, 'n');
}

void SurfaceImpl::SetClip(PRectangle rc) 
{
  // we deal with this in the renderer
}

void SurfaceImpl::FlushCachedState()
{}

void SurfaceImpl::SetUnicodeMode( bool mode )
{
  unicodeMode = mode;
}

void SurfaceImpl::SetDBCSMode( int cp )
{
  codePage = cp;
}

Surface *Surface::Allocate(int technology) 
{
  return new SurfaceImpl;
}

//////////////////////////////////////////////////////////////////////////
// Window

Window::~Window()
{
}

void Window::Destroy()
{
}

bool Window::HasFocus()
{
  return false;
}

std::map<Scintilla::WindowID,Scintilla::PRectangle> rects;
PRectangle Window::GetPosition() 
{
  return rects[wid];
}

void Window::SetPosition(PRectangle rc) 
{
  rects[wid] = rc;
}

void Window::SetPositionRelative(PRectangle rc, Window w)
{
}

PRectangle Window::GetClientPosition() 
{
  return PRectangle( 0, 0, rects[wid].Width(), rects[wid].Height() );
}

void Window::Show(bool show)
{
}

void Window::InvalidateAll()
{
}

void Window::InvalidateRectangle(PRectangle rc)
{
}

void Window::SetFont(Font &font)
{
}

void Window::SetCursor(Cursor curs) 
{
}

PRectangle Window::GetMonitorRect(Point pt)
{
  return PRectangle( 0, 0, Renderer::nWidth, Renderer::nHeight );
}

//////////////////////////////////////////////////////////////////////////
// Menus

Menu::Menu() : mid(0)
{
  assert(!"Implemented");
}

void Menu::CreatePopUp()
{
  assert(!"Implemented");
}

void Menu::Destroy()
{
  assert(!"Implemented");
}

void Menu::Show(Point pt, Window &w)
{
  assert(!"Implemented");
}

//////////////////////////////////////////////////////////////////////////
// ListBox implementation

ListBox *ListBox::Allocate()
{
  return NULL;
}
