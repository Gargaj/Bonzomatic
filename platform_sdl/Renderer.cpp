#ifdef _WIN32
#include <windows.h>
#endif
#include <sdl.h>
#include <GLee.h>
#include <gl\glu.h>
#include "../Renderer.h"

#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>

namespace Renderer
{
  SDL_Surface * mScreen = NULL;
  bool run = true;

  GLuint theShader = NULL;

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

    mScreen = SDL_SetVideoMode( settings->nWidth, settings->nHeight, 32, flags );
    if (!mScreen)
    {
      SDL_Quit();
      return false;
    }

    SDL_EnableUNICODE(TRUE);
    SDL_EnableKeyRepeat(250, 20);

/*
    SDL_SysWMinfo info = {{0, 0}, 0, 0};
    SDL_GetWMInfo(&info);
*/

    run = true;

    return true;
  }

  int textureUnit = 0;
  void StartFrame()
  {
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

//         if (E.key.keysym.sym==SDLK_F11)
//         {
//           visible = !visible;
//        app.handleKeyDown(E.key);
      }
      else if (E.type == SDL_MOUSEBUTTONDOWN)
      {
//        app.handleMouseDown(E.button);
      }
    }
    glClearColor(0.08f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    textureUnit = 1;
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

    Texture * tex = new Texture();
    tex->width = width;
    tex->height = height;
    tex->ID = (void *)glTexId;
    tex->type = TEXTURETYPE_2D;
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

    Texture * tex = new Texture();
    tex->width = w;
    tex->height = 1;
    tex->ID = (void *)glTexId;
    tex->type = TEXTURETYPE_1D;
    return tex;
  }

  void SetShaderTexture( char * szTextureName, Texture * tex )
  {
    GLint location = glGetUniformLocation( theShader, szTextureName );
    if ( location != -1 )
    {
      glProgramUniform1iEXT( theShader, location, textureUnit );
      glActiveTexture( GL_TEXTURE0 + textureUnit );
      switch( tex->type)
      {
        case TEXTURETYPE_1D: glBindTexture( GL_TEXTURE_1D, (GLuint)tex->ID ); break;
        case TEXTURETYPE_2D: glBindTexture( GL_TEXTURE_2D, (GLuint)tex->ID ); break;
      }
      textureUnit++;
    }
  }

  bool UpdateR32Texture( Texture * tex, float * data )
  {
    int unit = 1; // FIXME
    glActiveTexture( GL_TEXTURE0 + unit );
    glBindTexture( GL_TEXTURE_1D, (GLuint)tex->ID );
    glTexSubImage1D( GL_TEXTURE_1D, 0, 0, tex->width, GL_RED, GL_FLOAT, data );

    return true;
  }

}