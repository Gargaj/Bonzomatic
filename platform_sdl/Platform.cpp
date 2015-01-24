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
#include <glee.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "..\Renderer.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

//////////////////////////////////////////////////////////////////////////

ColourDesired MakeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a=0xFF)
{
  return a<<24|b<<16|g<<8|r;
}

ColourDesired Platform::Chrome() {
  return MakeRGBA(0xe0, 0xe0, 0xe0);
}

ColourDesired Platform::ChromeHighlight() {
  return MakeRGBA(0xff, 0xff, 0xff);
}

const char *Platform::DefaultFont() {
  return "Lucida Console";
}

int Platform::DefaultFontSize() {
  return 10;
}

unsigned int Platform::DoubleClickTime() {
  return 500; 	// Half a second
}

bool Platform::MouseButtonBounce() {
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
void Platform::DebugPrintf(const char *format, ...) {
  char buffer[2000];
  va_list pArguments;
  va_start(pArguments, format);
  vsprintf(buffer,format,pArguments);
  va_end(pArguments);
  Platform::DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...) {
}
#endif

//////////////////////////////////////////////////////////////////////////
// FONT


struct stbtt_Font
{
  stbtt_fontinfo fontinfo;
  stbtt_bakedchar cdata[512]; // ASCII 32..126 is 95 glyphs
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

  stbtt_BakeFontBitmap(buf, 0, fp.size, bmp, texSize, texSize, 0, 512, newFont->cdata); // no guarantee this fits!

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
  void InitPixMap(int width, int height, Surface *surface_, WindowID wid);

  void Release();
  bool Initialised();
  void PenColour(ColourDesired fore);
  int LogPixelsY();
  int DeviceHeightFont(int points);
  void MoveTo(float x_, float y_);
  void LineTo(float x_, float y_);
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

  void DrawTextBase(PRectangle rc, Font &font_, float ybase, const char *s, int len, ColourDesired fore);
  void DrawTextNoClip(PRectangle rc, Font &font_, float ybase, const char *s, int len, ColourDesired fore, ColourDesired back);
  void DrawTextClipped(PRectangle rc, Font &font_, float ybase, const char *s, int len, ColourDesired fore, ColourDesired back);
  void DrawTextTransparent(PRectangle rc, Font &font_, float ybase, const char *s, int len, ColourDesired fore);
  void MeasureWidths(Font &font_, const char *s, int len, float *positions);
  float WidthText(Font &font_, const char *s, int len);
  float WidthChar(Font &font_, char ch);
  float Ascent(Font &font_);
  float Descent(Font &font_);
  float InternalLeading(Font &font_);
  float ExternalLeading(Font &font_);
  float Height(Font &font_);
  float AverageCharWidth(Font &font_);

  void MoveTo(int x_, int y_);
  void LineTo(int x_, int y_);

  void SetClip(PRectangle rc);
  void FlushCachedState();

  void SetUnicodeMode(bool unicodeMode_);
  void SetDBCSMode(int codePage);
};

#ifdef SCI_NAMESPACE
}
#endif

SurfaceImpl::SurfaceImpl() : currentX(0), currentY(0) {
  unicodeMode = false;
  codePage = 0;
  initialised = false;
}

SurfaceImpl::~SurfaceImpl() {
}

void SurfaceImpl::Init( WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::Init( SurfaceID sid, WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::InitPixMap( int width, int height, Surface *surface_, WindowID wid )
{
  initialised = true;
}

void SurfaceImpl::Release() {
}

bool SurfaceImpl::Initialised()
{
  return initialised;
}

void SurfaceImpl::PenColour(ColourDesired fore) {
  penColour = fore;
}

int SurfaceImpl::LogPixelsY() {
  return 72;
}

int SurfaceImpl::DeviceHeightFont(int points) {
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
  glColor4ubv((GLubyte*)&penColour);
  glBegin(GL_LINES);
  glVertex2f(currentX+0.5f, currentY+0.5f);
  glVertex2f(targetX+0.5f, targetY+0.5f);
  glEnd();
  currentX = targetX;
  currentY = targetY;
}

void SurfaceImpl::Polygon(Point* /*pts*/, int /*npts*/, ColourDesired /*fore*/, ColourDesired /*back*/) 
{
  assert(0);
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

struct pixmap_t
{
  GLuint tex;
  float scalex, scaley;
  bool initialised;
};

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
  assert(!"Implemented");
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourDesired back) 
{
  Renderer::BindTexture(NULL);
  glDisable(GL_TEXTURE_2D);

  glColor4ubv((GLubyte*)&back);
  glBegin(GL_QUADS);
  glVertex2f(rc.left,  rc.top);
  glVertex2f(rc.right, rc.top);
  glVertex2f(rc.right, rc.bottom);
  glVertex2f(rc.left,  rc.bottom);
  glEnd();
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface & surfacePattern) {
  //assert(0);
  FillRectangle( rc, 0xd0000000 );
}

void SurfaceImpl::RoundedRectangle(PRectangle /*rc*/, ColourDesired /*fore*/, ColourDesired /*back*/) {
  assert(0);
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int /*cornerSize*/, ColourDesired fill, int alphaFill,
    ColourDesired /*outline*/, int /*alphaOutline*/, int /*flags*/) 
{
  unsigned int back = fill.AsLong() & 0xFFFFFF | ((alphaFill & 0xFF)<<24);
  FillRectangle(rc, back);
}

void SurfaceImpl::Ellipse(PRectangle /*rc*/, ColourDesired /*fore*/, ColourDesired /*back*/) {
  assert(0);
}

void SurfaceImpl::Copy( PRectangle rc, Point from, Surface &surfaceSource )
{
  //assert(0);
}

void SurfaceImpl::DrawTextBase(PRectangle rc, Font &font_, float ybase, const char *str, int len, ColourDesired fore) 
{
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();

  glEnable(GL_TEXTURE_2D);

  Renderer::BindTexture( realFont->texture );
  glColor3ubv((GLubyte*)&fore);
  glBegin(GL_QUADS);
  float x = rc.left, y=ybase;
  while (len-- > 0) 
  {
    unsigned int c = *str;
    unsigned int charLength = UTF8CharLength(c);
    if (charLength > 1)
    {
      c = 0;
      UTF16FromUTF8( str, charLength, (wchar_t*)&c, sizeof(unsigned int) );
    }
    stbtt_aligned_quad quad;
    stbtt_GetBakedQuad( realFont->cdata, realFont->texture->width, realFont->texture->height, c, &x, &y, &quad, 1 );
    
    glTexCoord2f(quad.s0,quad.t0); glVertex2f(quad.x0,quad.y0);
    glTexCoord2f(quad.s1,quad.t0); glVertex2f(quad.x1,quad.y0);
    glTexCoord2f(quad.s1,quad.t1); glVertex2f(quad.x1,quad.y1);
    glTexCoord2f(quad.s0,quad.t1); glVertex2f(quad.x0,quad.y1);
    str += charLength;
    len -= charLength - 1;
  }
  glEnd();
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, float ybase, const char *s, int len,
                                 ColourDesired fore, ColourDesired /*back*/) {
  DrawTextBase(rc, font_, ybase, s, len, fore);
}

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, float ybase, const char *s, int len,
                                  ColourDesired fore, ColourDesired /*back*/) {
  DrawTextBase(rc, font_, ybase, s, len, fore);
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, float ybase, const char *s, int len,
                                  ColourDesired fore) {
  DrawTextBase(rc, font_, ybase, s, len, fore);
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *str, int len, float *positions) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  //TODO: implement proper UTF-8 handling
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

    int advance, leftBearing;
    
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    
    position     += advance;
    for (int i=0; i<charLength; i++) // we need to loop here because UTF8 characters count as multiple unless their position is the same
      *positions++  = position*realFont->scale;

    p += charLength;
    len -= charLength - 1;
  }
}

float SurfaceImpl::WidthText(Font &font_, const char *str, int len) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  //TODO: implement proper UTF-8 handling
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
    int advance, leftBearing;
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    position += advance*realFont->scale;//TODO: +Kerning

    str += charLength;
    len -= charLength - 1;
  }
  return position;
}

float SurfaceImpl::WidthChar(Font &font_, char ch) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  int advance, leftBearing;
  stbtt_GetCodepointHMetrics(&realFont->fontinfo, ch, &advance, &leftBearing);
  return advance*realFont->scale;
}

float SurfaceImpl::Ascent(Font &font_) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return ascent*realFont->scale;
}

float SurfaceImpl::Descent(Font &font_) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return -descent*realFont->scale;
}

float SurfaceImpl::InternalLeading(Font &) {
  //WTF is this?????
  return 0;
}

float SurfaceImpl::ExternalLeading(Font& font_) {
  //WTF is this?????
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&realFont->fontinfo, &ascent, &descent, &lineGap);
  return lineGap*realFont->scale;
}

float SurfaceImpl::Height(Font &font_) {
  return Ascent(font_) + Descent(font_);
}

float SurfaceImpl::AverageCharWidth(Font &font_) 
{
  return WidthChar(font_, 'n');
}

void SurfaceImpl::MoveTo( int x_, int y_ )
{
  assert(0);
}

void SurfaceImpl::LineTo( int x_, int y_ )
{
  assert(0);
}

void SurfaceImpl::SetClip(PRectangle rc) 
{
  // we deal with this in the renderer
}

void SurfaceImpl::FlushCachedState() {}

void SurfaceImpl::SetUnicodeMode( bool unicodeMode_ )
{
  unicodeMode = unicodeMode_;
}

void SurfaceImpl::SetDBCSMode( int _codePage )
{
  codePage = _codePage;
}

Surface *Surface::Allocate(int technology) 
{
  return new SurfaceImpl;
}

//////////////////////////////////////////////////////////////////////////
// Dynamic libraries

class DynamicLibraryImpl : public DynamicLibrary {
protected:
  HMODULE h;
public:
  explicit DynamicLibraryImpl(const char *modulePath) {
    h = ::LoadLibraryA(modulePath);
  }

  virtual ~DynamicLibraryImpl() {
    if (h != NULL)
      ::FreeLibrary(h);
  }

  // Use GetProcAddress to get a pointer to the relevant function.
  virtual Function FindFunction(const char *name) {
    if (h != NULL) {
      // C++ standard doesn't like casts betwen function pointers and void pointers so use a union
      union {
        FARPROC fp;
        Function f;
      } fnConv;
      fnConv.fp = ::GetProcAddress(h, name);
      return fnConv.f;
    } else {
      return NULL;
    }
  }

  virtual bool IsValid() {
    return h != NULL;
  }
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath) {
  return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
}

//////////////////////////////////////////////////////////////////////////
// Window

Window::~Window() {
}

void Window::Destroy() {
}

bool Window::HasFocus() {
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

void Window::SetPositionRelative(PRectangle rc, Window w) {
}

PRectangle Window::GetClientPosition() 
{
  return PRectangle( 0, 0, rects[wid].Width(), rects[wid].Height() );
}

void Window::Show(bool show) {
}

void Window::InvalidateAll() {
}

void Window::InvalidateRectangle(PRectangle rc) {
}

void Window::SetFont(Font &font) {
}

void Window::SetCursor(Cursor curs) 
{
}

PRectangle Window::GetMonitorRect(Point pt) {
  return PRectangle( 0, 0, Renderer::nWidth, Renderer::nHeight );
}

//////////////////////////////////////////////////////////////////////////
// Menus

Menu::Menu() : mid(0) {
  assert(0);
}

void Menu::CreatePopUp() {
  assert(0);
}

void Menu::Destroy() {
  assert(0);
}

void Menu::Show(Point pt, Window &w) {
  assert(0);
}

//////////////////////////////////////////////////////////////////////////
// ListBox implementation

ListBox *ListBox::Allocate() {
  return NULL;
}
