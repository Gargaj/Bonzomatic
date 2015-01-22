#ifdef _WIN32
#include <windows.h>
#endif
#include <sdl.h>
#include <gl/gl.h>
#include "../Renderer.h"

namespace Renderer
{
  SDL_Surface* mScreen;
  bool run = true;

  bool Open( RENDERER_SETTINGS * settings )
  {
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
}