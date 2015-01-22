// ScintillaGL source code edit control
// PlatGTK.cxx - implementation of platform facilities on GTK+/Linux
// Copyright 2011 by Mykhailo Parfeniuk
// The License.txt file describes the conditions under which this software may be distributed.

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

void Platform::Assert(const char *c, const char *file, int line) {
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
// PLATFORM

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif
class SurfaceImpl : public Surface {
  ColourDesired penColour;
  float x;
  float y;
  bool unicodeMode;
  int codePage;
  bool initialised;
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

SurfaceImpl::SurfaceImpl() : x(0), y(0) {
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

void SurfaceImpl::MoveTo(float x_, float y_) {
  x = x_;
  y = y_;
}

void SurfaceImpl::LineTo(float x_, float y_) {
  glColor4ubv((GLubyte*)&penColour);
  glBegin(GL_LINES);
  glVertex2f(x+0.5f,  y+0.5f);
  glVertex2f(x_+0.5f, y_+0.5f);
  glEnd();
  x = x_;
  y = y_;
}

void SurfaceImpl::Polygon(Point* /*pts*/, int /*npts*/, ColourDesired /*fore*/,
                          ColourDesired /*back*/) {
  assert(0);
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) 
{
  FillRectangle(rc, back);
  glColor4ubv((GLubyte*)&fore);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINE_STRIP);
  glVertex2f(rc.left+0.5f,  rc.top+0.5f);
  glVertex2f(rc.right-0.5f, rc.top+0.5f);
  glVertex2f(rc.right-0.5f, rc.bottom-0.5f);
  glVertex2f(rc.left+0.5f,  rc.bottom-0.5f);
  glVertex2f(rc.left+0.5f,  rc.top+0.5f);
  glEnd();
}

struct pixmap_t
{
  GLuint tex;
  float scalex, scaley;
  bool initialised;
};

/*
Pixmap	CreatePixmap()
{
  Pixmap pm = new pixmap_t;
  pm->scalex = 0;
  pm->scaley = 0;
  pm->initialised = false;

  return pm;
}

bool	IsPixmapInitialised(Pixmap pixmap)
{
  return pixmap->initialised;
}

void	DestroyPixmap(Pixmap pixmap)
{
  glDeleteTextures(1, &pixmap->tex);
  delete pixmap;
}

void	UpdatePixmap(Pixmap pixmap, int w, int h, int* data)
{
  if (!pixmap->initialised)
  {
    glGenTextures(1, &pixmap->tex);
    glBindTexture(GL_TEXTURE_2D, pixmap->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, pixmap->tex);
  }

  pixmap->initialised = true;
  pixmap->scalex = 1.0f/w;
  pixmap->scaley = 1.0f/h;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);

}
*/

/*
void SurfaceImpl::DrawPixmap(PRectangle rc, Point offset, Pixmap pixmap)
{
  float w = (rc.right-rc.left)*pixmap->scalex, h=(rc.bottom-rc.top)*pixmap->scaley;
  float u1 = offset.x*pixmap->scalex, v1 = offset.y*pixmap->scaley, u2 = u1+w, v2 = v1+h;

  for (int i=0; i<8; i++)
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture(GL_TEXTURE_2D, NULL);
  }
  glActiveTexture( GL_TEXTURE0 );

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, pixmap->tex);
  glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
  glBegin(GL_QUADS);
  glTexCoord2f(u1, v1);
  glVertex2f(rc.left,  rc.top);
  glTexCoord2f(u2, v1);
  glVertex2f(rc.right, rc.top);
  glTexCoord2f(u2, v2);
  glVertex2f(rc.right, rc.bottom);
  glTexCoord2f(u1, v2);
  glVertex2f(rc.left,  rc.bottom);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}
*/

void SurfaceImpl::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
    assert(!"Implemented");
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourDesired back) {
  for (int i=0; i<8; i++)
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture(GL_TEXTURE_2D, NULL);
  }
  glActiveTexture( GL_TEXTURE0 );

  glColor4ubv((GLubyte*)&back);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glVertex2f(rc.left,  rc.top);
  glVertex2f(rc.right, rc.top);
  glVertex2f(rc.right, rc.bottom);
  glVertex2f(rc.left,  rc.bottom);
  glEnd();
}

void SurfaceImpl::FillRectangle(PRectangle /*rc*/, Surface &/*surfacePattern*/) {
  assert(0);
}

void SurfaceImpl::RoundedRectangle(PRectangle /*rc*/, ColourDesired /*fore*/, ColourDesired /*back*/) {
  assert(0);
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int /*cornerSize*/, ColourDesired fill, int alphaFill,
    ColourDesired /*outline*/, int /*alphaOutline*/, int /*flags*/) {
  unsigned int back = fill.AsLong()&0xFFFFFF | ((alphaFill&0xFF)<<24);
  glDisable(GL_TEXTURE_2D);
  glColor4ubv((GLubyte*)&back);
  glBegin(GL_QUADS);
  glVertex2f(rc.left,  rc.top);
  glVertex2f(rc.right, rc.top);
  glVertex2f(rc.right, rc.bottom);
  glVertex2f(rc.left,  rc.bottom);
  glEnd();
}

void SurfaceImpl::Ellipse(PRectangle /*rc*/, ColourDesired /*fore*/, ColourDesired /*back*/) {
  assert(0);
}

void SurfaceImpl::Copy( PRectangle rc, Point from, Surface &surfaceSource )
{
  assert(0);
}

struct stbtt_Font
{
  stbtt_fontinfo	fontinfo;
  stbtt_bakedchar cdata[512]; // ASCII 32..126 is 95 glyphs
  GLuint ftex;
  float scale;
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
  size_t len;

  FILE* f = fopen(fp.faceName, "rb");

  assert(f);

  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);

  unsigned char* buf = (unsigned char*)malloc(len);
  unsigned char* bmp = new unsigned char[1024*1024];
  fread(buf, 1, len, f);
  stbtt_BakeFontBitmap(buf, 0, fp.size, bmp, 1024, 1024, 0, 512, newFont->cdata); // no guarantee this fits!

  FILE* dump = fopen("font.raw", "wb");
  fwrite(bmp,1024,1024,dump);
  fclose(dump);

  // can free ttf_buffer at this point
  glGenTextures(1, &newFont->ftex);
  glBindTexture(GL_TEXTURE_2D, newFont->ftex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1024, 1024, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmp);
  // can free temp_bitmap at this point
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  fclose(f);

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
    glDeleteTextures(1, &((stbtt_Font*)fid)->ftex);
    delete (stbtt_Font*)fid;
  }
}

void SurfaceImpl::DrawTextBase(PRectangle rc, Font &font_, float ybase, const char *s, int len,
                                 ColourDesired fore) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();

//   GLint prevActiveTexUnit;
//   glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTexUnit
  for (int i=0; i<8; i++)
  {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture(GL_TEXTURE_2D, NULL);
  }
  glActiveTexture( GL_TEXTURE0 );

  glEnable(GL_TEXTURE_2D);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // assume orthographic projection with units = screen pixels, origin at top left
  glBindTexture(GL_TEXTURE_2D, realFont->ftex);
  glColor3ubv((GLubyte*)&fore);
  glBegin(GL_QUADS);
  float x = rc.left, y=ybase;
  while (len--) 
  {
    //if (*s >= 32 && *s < 128) 
    unsigned int c = *s;
    unsigned int l = UTF8CharLength(c);
    if (l > 1)
    {
      c = 0;
      UTF16FromUTF8( s, l, (wchar_t*)&c, sizeof(unsigned int) );
    }
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(realFont->cdata, 1024,1024, c, &x,&y,&q,1);//1=opengl,0=old d3d
    //x = floor(x);
    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
    s += l;
    len -= l - 1;
  }
  glEnd();
  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_BLEND);
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

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len, float *positions) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  //TODO: implement proper UTF-8 handling
  float position = 0;
  while (len--) 
  {
    unsigned int c = *s;
    unsigned int l = UTF8CharLength(c);
    if (l > 1)
    {
      c = 0;
      UTF16FromUTF8( s, l, (wchar_t*)&c, sizeof(unsigned int) );
    }

    int advance, leftBearing;
    
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    
    position     += advance;//TODO: +Kerning
    *positions++  = position*realFont->scale;

    s += l;
    len -= l - 1;
  }
}

float SurfaceImpl::WidthText(Font &font_, const char *s, int len) {
  stbtt_Font* realFont = (stbtt_Font*)font_.GetID();
  //TODO: implement proper UTF-8 handling
  float position = 0;
  while (len--) 
  {
    unsigned int c = *s;
    unsigned int l = UTF8CharLength(c);
    if (l > 1)
    {
      c = 0;
      UTF16FromUTF8( s, l, (wchar_t*)&c, sizeof(unsigned int) );
    }
    int advance, leftBearing;
    stbtt_GetCodepointHMetrics(&realFont->fontinfo, c, &advance, &leftBearing);
    position += advance*realFont->scale;//TODO: +Kerning

    s += l;
    len -= l - 1;
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

void SurfaceImpl::SetClip(PRectangle rc) {
  double plane[][4] = {
    { 1,  0, 0, -rc.left  },
    {-1,  0, 0,  rc.right },
    { 0,  1, 0, -rc.top   },
    { 0, -1, 0,  rc.bottom},
  };
  glClipPlane(GL_CLIP_PLANE0, plane[0]);
  glClipPlane(GL_CLIP_PLANE1, plane[1]);
  glClipPlane(GL_CLIP_PLANE2, plane[2]);
  glClipPlane(GL_CLIP_PLANE3, plane[3]);
  //assert(0);
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

PRectangle Window::GetPosition() {
  return PRectangle();
}

void Window::SetPosition(PRectangle rc) {
}

void Window::SetPositionRelative(PRectangle rc, Window w) {
}

PRectangle Window::GetClientPosition() {
  return PRectangle();
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
  switch (curs) {
  case cursorText:
    ::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
    break;
  case cursorUp:
    ::SetCursor(::LoadCursor(NULL,IDC_UPARROW));
    break;
  case cursorWait:
    ::SetCursor(::LoadCursor(NULL,IDC_WAIT));
    break;
  case cursorHoriz:
    ::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
    break;
  case cursorVert:
    ::SetCursor(::LoadCursor(NULL,IDC_SIZENS));
    break;
  case cursorHand:
    ::SetCursor(::LoadCursor(NULL,IDC_HAND));
    break;
  case cursorReverseArrow:
    //::SetCursor(GetReverseArrowCursor());
    break;
  case cursorArrow:
  case cursorInvalid:	// Should not occur, but just in case.
    ::SetCursor(::LoadCursor(NULL,IDC_ARROW));
    break;
  }
}

PRectangle Window::GetMonitorRect(Point pt) {
  return PRectangle();
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
