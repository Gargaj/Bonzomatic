#ifdef _WIN32
#include <windows.h>
#endif
#include <sdl.h>
#include <GLee.h>
#include <gl\glu.h>
#include "../Renderer.h"

#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>
#include "../external/scintilla/include/Scintilla.h"

namespace Renderer
{
  char defaultShader[65536] = 
    "#version 430 core\n"
    "\n"
    "///////////////////////////////////////////////////////////////////////////////\n"
    "// shader inputs/outputs\n"
    "uniform float fGlobalTime; // in seconds\n"
    "uniform vec2 v2Resolution; // viewport resolution (in pixels) (1080p or 720p)\n"
    //   "uniform mat4 iMidiPad; // 16 buttons of midi controller\n"
    //   "uniform float iMidiPadValue; // sum of all elements in iMidiPad/16\n"
    "\n"
    "// all samplers have linear filtering applied, wrapping set to repeat\n"
    "uniform sampler1D texFFT; // 1024\n"
    //  "uniform float iFFT[8]; // latest frame\n"
    //  "uniform float iFFTs[8]; // smoothed latest frame\n"
    //  "uniform sampler2D iFFTsHistory; // smoothed fft history, 8x1024, x coord = bin, y coord n-frames earlier, y=0 is latest frame\n"
    "\n"
    "// predefined textures\n"
    "uniform sampler2D texTex1;\n"
    "uniform sampler2D texTex2;\n"
    "uniform sampler2D texTex3;\n"
    "uniform sampler2D texTex4;\n"
    "uniform sampler2D texNoise;\n"
    "uniform sampler2D texChecker;\n"
    "\n"
    "// out_color must be written in order to see anything\n"
    "layout(location = 0) out vec4 out_color;\n"
    "///////////////////////////////////////////////////////////////////////////////\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "  vec2 uv = vec2( gl_FragCoord.xy ) / v2Resolution;\n"
    "  uv -= 0.5;\n"
    "  uv /= vec2(v2Resolution.y / v2Resolution.x, 1);\n"
    "\n"
    "  vec2 m;\n"
    "  m.x = atan(uv.x / uv.y) / 3.14;\n"
    "  m.y = 1 / length(uv) * .2;\n"
    "  float d = m.y;\n"
    "\n"
    "  float f = texture( texFFT, d ).r * 100;\n"
    "  m.x += sin( fGlobalTime ) * 0.1;\n"
    "  m.y += fGlobalTime * 0.25;\n"
    "\n"
    "  vec4 t = texture( texTex2, m.xy ) * d; // or /d\n"
    "  out_color = f + t;// + uv.xyxy * 0.5 * (sin( fGlobalTime ) + 1.5);\n"
    "}";

  SDL_Surface * mScreen = NULL;
  bool run = true;

  GLuint theShader = NULL;

  int nWidth = 0;
  int nHeight = 0;
  bool Open( RENDERER_SETTINGS * settings )
  {
    theShader = NULL;
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0)
    {
      return false;
    }

    uint32_t flags = SDL_HWSURFACE|SDL_OPENGLBLIT;
    if (settings->windowMode == RENDERER_WINDOWMODE_FULLSCREEN)
      flags |= SDL_FULLSCREEN;

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, true );

    nWidth = settings->nWidth;
    nHeight = settings->nHeight;
    mScreen = SDL_SetVideoMode( settings->nWidth, settings->nHeight, 32, flags );
    if (!mScreen)
    {
      SDL_Quit();
      return false;
    }

    SDL_EnableUNICODE(TRUE);
    SDL_EnableKeyRepeat(250, 20);

    run = true;

    return true;
  }

  KeyEvent keyEventBuffer[512];
  int keyEventBufferCount = 0;
  MouseEvent mouseEventBuffer[512];
  int mouseEventBufferCount = 0;
  void StartFrame()
  {
    keyEventBufferCount = 0;
    mouseEventBufferCount = 0;
    SDL_Event	E;
    while (SDL_PollEvent(&E))
    {
      if (E.type == SDL_QUIT) 
      {
        run = false;
      }
      else if (E.type == SDL_KEYDOWN)
      {
        if (E.key.keysym.sym == SDLK_F4 && (E.key.keysym.mod == KMOD_LALT || E.key.keysym.mod == KMOD_RALT)) 
        {
          run = false;
        }
        int sciKey;
        switch(E.key.keysym.sym)
        {
          case SDLK_DOWN:         sciKey = SCK_DOWN;      break;
          case SDLK_UP:           sciKey = SCK_UP;        break;
          case SDLK_LEFT:         sciKey = SCK_LEFT;      break;
          case SDLK_RIGHT:        sciKey = SCK_RIGHT;     break;
          case SDLK_HOME:         sciKey = SCK_HOME;      break;
          case SDLK_END:          sciKey = SCK_END;       break;
          case SDLK_PAGEUP:       sciKey = SCK_PRIOR;     break;
          case SDLK_PAGEDOWN:     sciKey = SCK_NEXT;      break;
          case SDLK_DELETE:       sciKey = SCK_DELETE;    break;
          case SDLK_INSERT:       sciKey = SCK_INSERT;    break;
          case SDLK_ESCAPE:       sciKey = SCK_ESCAPE;    break;
          case SDLK_BACKSPACE:    sciKey = SCK_BACK;      break;
          case SDLK_TAB:          sciKey = SCK_TAB;       break;
          case SDLK_RETURN:       sciKey = SCK_RETURN;    break;
          case SDLK_KP_PLUS:      sciKey = SCK_ADD;       break;
          case SDLK_KP_MINUS:     sciKey = SCK_SUBTRACT;  break;
          case SDLK_KP_DIVIDE:    sciKey = SCK_DIVIDE;    break;
          case SDLK_LSUPER:       sciKey = SCK_WIN;       break;
          case SDLK_RSUPER:       sciKey = SCK_RWIN;      break;
          case SDLK_MENU:         sciKey = SCK_MENU;      break;
          case SDLK_SLASH:        sciKey = '/';           break;
          case SDLK_ASTERISK:     sciKey = '`';           break;
          case SDLK_LEFTBRACKET:  sciKey = '[';           break;
          case SDLK_BACKSLASH:    sciKey = '\\';          break;
          case SDLK_RIGHTBRACKET: sciKey = ']';           break;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
          case SDLK_LALT:
          case SDLK_RALT:
          case SDLK_LCTRL:
          case SDLK_RCTRL:
            sciKey = 0;
            break;
          default:
            sciKey = E.key.keysym.sym;
        }

        if (sciKey)
        {
          keyEventBuffer[keyEventBufferCount].ctrl  = E.key.keysym.mod & KMOD_LCTRL  || E.key.keysym.mod & KMOD_RCTRL;
          keyEventBuffer[keyEventBufferCount].alt   = E.key.keysym.mod & KMOD_LALT   || E.key.keysym.mod & KMOD_RALT;
          keyEventBuffer[keyEventBufferCount].shift = E.key.keysym.mod & KMOD_LSHIFT || E.key.keysym.mod & KMOD_RSHIFT;
          keyEventBuffer[keyEventBufferCount].scanCode = sciKey;
          keyEventBuffer[keyEventBufferCount].character = E.key.keysym.unicode;
          keyEventBufferCount++;
        }

      }
      else if (E.type == SDL_MOUSEBUTTONDOWN)
      {
        mouseEventBuffer[mouseEventBufferCount].x = E.button.x;
        mouseEventBuffer[mouseEventBufferCount].y = E.button.y;
        switch(E.button.button)
        {
          case SDL_BUTTON_MIDDLE: mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_MIDDLE; break;
          case SDL_BUTTON_RIGHT:  mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_RIGHT; break;
          case SDL_BUTTON_LEFT:   
          default:                mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT; break;
        }
        mouseEventBufferCount++;
      }
    }
    glClearColor(0.08f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  }
  void EndFrame()
  {
    SDL_GL_SwapBuffers();
  }
  bool WantsToQuit()
  {
    return !run;
  }
  void Close()
  {
    SDL_Quit();
  }

  void RenderFullscreenQuad()
  {
    glUseProgram(theShader);
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.00f, -1.00f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.00f, -1.00f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.00f,  1.00f);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.00f,  1.00f);
    glEnd();
    glUseProgram(0);
  }

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    GLuint prg = glCreateProgram();
    GLuint shd = glCreateShader(GL_FRAGMENT_SHADER);
    GLint size = 0;
    GLint result = 0;

    glShaderSource(shd, 1, (const GLchar**)&szShaderCode, &nShaderCodeSize);
    glCompileShader(shd);
    glGetShaderInfoLog(shd, nErrorBufferSize, &size, szErrorBuffer);
    glGetShaderiv(shd, GL_COMPILE_STATUS, &result);
    if (!result)
    {
      glDeleteProgram(prg);
      glDeleteShader(shd);
      return false;
    }

    glAttachShader(prg, shd);
    glLinkProgram(prg);
    glGetProgramInfoLog(prg, nErrorBufferSize - size, &size, szErrorBuffer + size);
    glGetProgramiv(prg, GL_LINK_STATUS, &result);
    if (!result)
    {
      glDeleteProgram(prg);
      glDeleteShader(shd);
      return false;
    }

    glDeleteProgram(theShader);

    theShader = prg;

    return true;
  }

  void SetShaderConstant( char * szConstName, float x )
  {
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
      glProgramUniform1fEXT( theShader, location, x );
    }
  }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
      glProgramUniform2fEXT( theShader, location, x, y );
    }
  }

  struct GLTexture : public Texture
  {
    GLuint ID;
    int unit;
  };

  int textureUnit = 0;
  Texture * CreateRGBA8TextureFromFile( char * szFilename )
  {
    int comp = 0;
    int width = 0;
    int height = 0;
    unsigned char * c = stbi_load( szFilename, (int*)&width, (int*)&height, &comp, STBI_rgb_alpha );
    if (!c) return NULL;

    GLuint glTexId = 0;
    glGenTextures( 1, &glTexId );
    glBindTexture( GL_TEXTURE_2D, glTexId );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    GLenum internalFormat = GL_SRGB8_ALPHA8;
    GLenum srcFormat = GL_RGBA;

    glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, srcFormat, GL_UNSIGNED_BYTE, c );

    stbi_image_free(c);

    GLTexture * tex = new GLTexture();
    tex->width = width;
    tex->height = height;
    tex->ID = glTexId;
    tex->type = TEXTURETYPE_2D;
    tex->unit = textureUnit++;
    return tex;
  }

  Texture * Create1DR32Texture( int w )
  {
    GLuint glTexId = 0;
    glGenTextures( 1, &glTexId );
    glBindTexture( GL_TEXTURE_1D, glTexId );

    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    float * data = new float[w];
    for ( int i = 0; i < w; ++i )
      data[i] = 1.0f;

    glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, w, 0, GL_RED, GL_FLOAT, data );

    delete[] data;

    glBindTexture( GL_TEXTURE_1D, 0 );

    GLTexture * tex = new GLTexture();
    tex->width = w;
    tex->height = 1;
    tex->ID = glTexId;
    tex->type = TEXTURETYPE_1D;
    tex->unit = textureUnit++;
    return tex;
  }

  void SetShaderTexture( char * szTextureName, Texture * tex )
  {
    if (!tex)
      return;

    GLint location = glGetUniformLocation( theShader, szTextureName );
    if ( location != -1 )
    {
      glProgramUniform1iEXT( theShader, location, ((GLTexture*)tex)->unit );
      glActiveTexture( GL_TEXTURE0 + ((GLTexture*)tex)->unit );
      switch( tex->type)
      {
        case TEXTURETYPE_1D: glBindTexture( GL_TEXTURE_1D, ((GLTexture*)tex)->ID ); break;
        case TEXTURETYPE_2D: glBindTexture( GL_TEXTURE_2D, ((GLTexture*)tex)->ID ); break;
      }
    }
  }

  bool UpdateR32Texture( Texture * tex, float * data )
  {
    glActiveTexture( GL_TEXTURE0 + ((GLTexture*)tex)->unit );
    glBindTexture( GL_TEXTURE_1D, ((GLTexture*)tex)->ID );
    glTexSubImage1D( GL_TEXTURE_1D, 0, 0, tex->width, GL_RED, GL_FLOAT, data );

    return true;
  }

  void StartTextRendering()
  {
    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, nWidth, 0, nHeight, 0, 500);
    glTranslatef(0, nHeight, 0);
    glScalef(1, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    for (int i=0; i<8; i++)
    {
      glActiveTexture( GL_TEXTURE0 + i );
      glBindTexture(GL_TEXTURE_2D, NULL);
    }
    glActiveTexture( GL_TEXTURE0 );
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  void SetTextRenderingViewport( Scintilla::PRectangle rect )
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor( rect.left, nHeight - rect.bottom, rect.right - rect.left, rect.bottom - rect.top );
    glLoadIdentity();
    glTranslatef( rect.left, rect.top, 0.0 );
  }
  void EndTextRendering()
  {
    glPopAttrib();
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
  }

  Texture * CreateA8TextureFromData( int w, int h, unsigned char * data )
  {
    GLuint glTexId = 0;
    glGenTextures(1, &glTexId);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLTexture * tex = new GLTexture();
    tex->width = w;
    tex->height = h;
    tex->ID = glTexId;
    tex->type = TEXTURETYPE_2D;
    tex->unit = 0; // this is always 0 cos we're not using shaders here
    return tex;
  }

  void ReleaseTexture( Texture * tex )
  {
    glDeleteTextures(1, &((GLTexture*)tex)->ID );
  }

  void BindTexture( Texture * tex )
  {
    if (tex) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex ? ((GLTexture*)tex)->ID : NULL );
  }

  void RenderQuad( Vertex & a, Vertex & b, Vertex & c, Vertex & d )
  {
    glBegin(GL_QUADS);
    glColor4ubv( (GLubyte*)&a.c ); glTexCoord2f( a.u, a.v ); glVertex2f( a.x, a.y );
    glColor4ubv( (GLubyte*)&b.c ); glTexCoord2f( b.u, b.v ); glVertex2f( b.x, b.y );
    glColor4ubv( (GLubyte*)&c.c ); glTexCoord2f( c.u, c.v ); glVertex2f( c.x, c.y );
    glColor4ubv( (GLubyte*)&d.c ); glTexCoord2f( d.u, d.v ); glVertex2f( d.x, d.y );
    glEnd();
  }

  void RenderLine( Vertex & a, Vertex & b )
  {
    glBegin(GL_LINES);
    glColor4ubv( (GLubyte*)&a.c ); glTexCoord2f( a.u, a.v ); glVertex2f( a.x, a.y );
    glColor4ubv( (GLubyte*)&b.c ); glTexCoord2f( b.u, b.v ); glVertex2f( b.x, b.y );
    glEnd();
  }

}