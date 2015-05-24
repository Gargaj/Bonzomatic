#ifdef _WIN32
#include <windows.h>
#endif
#include <SDL.h>
#include <GLee.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "../Renderer.h"

#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>
#include "../external/scintilla/include/Scintilla.h"

#include <iostream>

using namespace std;

const char * shaderKeyword =
  "discard struct if else switch case default break goto return for while do continue";

const char * shaderType = 
  "attribute const in inout out uniform varying invariant "
  "centroid flat smooth noperspective layout patch sample "
  "subroutine lowp mediump highp precision "
  "void float vec2 vec3 vec4 bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 "
  "uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 "
  "sampler1D sampler2D sampler3D isampler2D isampler1D isampler3D "
  "usampler1D usampler2D usampler3D "
  "sampler1DShadow sampler2DShadow sampler1DArray sampler2DArray "
  "sampler1DArrayShadow sampler2DArrayShadow "
  "samplerCube samplerCubeShadow samplerCubeArrayShadow ";

const char * shaderBuiltin = 
  "radians degrees sin cos tan asin acos atan sinh "
  "cosh tanh asinh acosh atanh pow exp log exp2 "
  "log2 sqrt inversesqrt abs sign floor trunc round "
  "roundEven ceil fract mod modf min max clamp mix "
  "step smoothstep isnan isinf floatBitsToInt floatBitsToUint "
  "intBitsToFloat uintBitsToFloat fma frexp ldexp packUnorm2x16 "
  "packUnorm4x8 packSnorm4x8 unpackUnorm2x16 unpackUnorm4x8 "
  "unpackSnorm4x8 packDouble2x32 unpackDouble2x32 length distance "
  "dot cross normalize ftransform faceforward reflect "
  "refract matrixCompMult outerProduct transpose determinant "
  "inverse lessThan lessThanEqual greaterThan greaterThanEqual "
  "equal notEqual any all not uaddCarry usubBorrow "
  "umulExtended imulExtended bitfieldExtract bitfildInsert "
  "bitfieldReverse bitCount findLSB findMSB textureSize "
  "textureQueryLOD texture textureProj textureLod textureOffset "
  "texelFetch texelFetchOffset textureProjOffset textureLodOffset "
  "textureProjLod textureProjLodOffset textureGrad textureGradOffset "
  "textureProjGrad textureProjGradOffset textureGather "
  "textureGatherOffset texture1D texture2D texture3D texture1DProj "
  "texture2DProj texture3DProj texture1DLod texture2DLod "
  "texture3DLod texture1DProjLod texture2DProjLod texture3DProjLod "
  "textureCube textureCubeLod shadow1D shadow2D shadow1DProj "
  "shadow2DProj shadow1DLod shadow2DLod shadow1DProjLod "
  "shadow2DProjLod dFdx dFdy fwidth interpolateAtCentroid "
  "interpolateAtSample interpolateAtOffset noise1 noise2 noise3 "
  "noise4 EmitStreamVertex EndStreamPrimitive EmitVertex "
  "EndPrimitive barrier "
  "gl_VertexID gl_InstanceID gl_Position gl_PointSize "
  "gl_ClipDistance gl_PrimitiveIDIn gl_InvocationID gl_PrimitiveID "
  "gl_Layer gl_PatchVerticesIn gl_TessLevelOuter gl_TessLevelInner "
  "gl_TessCoord gl_FragCoord gl_FrontFacing gl_PointCoord "
  "gl_SampleID gl_SamplePosition gl_FragColor gl_FragData "
  "gl_FragDepth gl_SampleMask gl_ClipVertex gl_FrontColor "
  "gl_BackColor gl_FrontSecondaryColor gl_BackSecondaryColor "
  "gl_TexCoord gl_FogFragCoord gl_Color gl_SecondaryColor "
  "gl_Normal gl_Vertex gl_MultiTexCoord0 gl_MultiTexCoord1 "
  "gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 "
  "gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_FogCoord "
  "gl_MaxVertexAttribs gl_MaxVertexUniformComponents gl_MaxVaryingFloats "
  "gl_MaxVaryingComponents gl_MaxVertexOutputComponents "
  "gl_MaxGeometryInputComponents gl_MaxGeometryOutputComponents "
  "gl_MaxFragmentInputComponents gl_MaxVertexTextureImageUnits "
  "gl_MaxCombinedTextureImageUnits gl_MaxTextureImageUnits "
  "gl_MaxFragmentUniformComponents gl_MaxDrawBuffers gl_MaxClipDistances "
  "gl_MaxGeometryTextureImageUnits gl_MaxGeometryOutputVertices "
  "gl_MaxGeometryTotalOutputComponents gl_MaxGeometryUniformComponents "
  "gl_MaxGeometryVaryingComponents gl_MaxTessControlInputComponents "
  "gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits "
  "gl_MaxTessControlUniformComponents "
  "gl_MaxTessControlTotalOutputComponents "
  "gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents "
  "gl_MaxTessEvaluationTextureImageUnits "
  "gl_MaxTessEvaluationUniformComponents gl_MaxTessPatchComponents "
  "gl_MaxPatchVertices gl_MaxTessGenLevel gl_MaxTextureUnits "
  "gl_MaxTextureCoords gl_MaxClipPlanes "
  "gl_DepthRange gl_ModelViewMatrix gl_ProjectionMatrix "
  "gl_ModelViewProjectionMatrix gl_TextureMatrix gl_NormalMatrix "
  "gl_ModelViewMatrixInverse gl_ProjectionMatrixInverse "
  "gl_ModelViewProjectionMatrixInverse gl_TextureMatrixInverse "
  "gl_ModelViewMatrixTranspose gl_ProjectionMatrixTranspose "
  "gl_ModelViewProjectionMatrixTranspose gl_TextureMatrixTranspose "
  "gl_ModelViewMatrixInverseTranspose gl_ProjectionMatrixInverseTranspose "
  "gl_ModelViewProjectionMatrixInverseTranspose "
  "gl_TextureMatrixInverseTranspose gl_NormalScale gl_ClipPlane "
  "gl_Point gl_FrontMaterial gl_BackMaterial gl_LightSource "
  "gl_LightModel gl_FrontLightModelProduct gl_BackLightModelProduct "
  "gl_FrontLightProduct gl_BackLightProduct gl_TextureEnvColor "
  "gl_EyePlaneS gl_EyePlaneT gl_EyePlaneR gl_EyePlaneQ "
  "gl_ObjectPlaneS gl_ObjectPlaneT gl_ObjectPlaneR gl_ObjectPlaneQ "
  "gl_Fog";

namespace Renderer
{
  char * defaultShaderFilename = "shader.glsl";
  char defaultShader[65536] =
#ifdef __APPLE__
 
#else
    "#version 430 core\n"
#endif
    "\n"
    "uniform float fGlobalTime; // in seconds\n"
    "uniform vec2 v2Resolution; // viewport resolution (in pixels)\n"
    "\n"
    "uniform sampler1D texFFT; // towards 0.0 is bass / lower freq, towards 1.0 is higher / treble freq\n"
    "uniform sampler1D texFFTSmoothed; // this one has longer falloff and less harsh transients\n"
    "{%textures:begin%}" // leave off \n here
    "uniform sampler2D {%textures:name%};\n"
    "{%textures:end%}" // leave off \n here
    "{%midi:begin%}" // leave off \n here
    "float {%midi:name%};\n"
    "{%midi:end%}" // leave off \n here
    "\n"
#ifdef __APPLE__
    "vec4 out_color; // out_color must be written in order to see anything\n"
#else
    "layout(location = 0) out vec4 out_color; // out_color must be written in order to see anything\n"
#endif
    "\n"
    "vec4 plas( vec2 v, float time )\n"
    "{\n"
    "  float c = 0.5 + sin( v.x * 10.0 ) + cos( sin( time + v.y ) * 20.0 );\n"
    "  return vec4( sin(c * 0.2 + cos(time)), c * 0.15, cos( c * 0.1 + time / .4 ) * .25, 1.0 );\n"
    "}\n"
    "void main(void)\n"
    "{\n"
    "  vec2 uv = vec2(gl_FragCoord.x / v2Resolution.x, gl_FragCoord.y / v2Resolution.y);\n"
    "  uv -= 0.5;\n"
    "  uv /= vec2(v2Resolution.y / v2Resolution.x, 1);\n"
    "\n"
    "  vec2 m;\n"
    "  m.x = atan(uv.x / uv.y) / 3.14;\n"
#ifdef __APPLE__
    "  m.y = 1.0;\n"
#else
    "  m.y = 1 / length(uv) * .2;\n"
#endif
    "  float d = m.y;\n"
    "\n"
#ifdef __APPLE__
    "  float f = texture1D( texFFT, d).r * 100.0;\n"
#else
    "  float f = texture( texFFT, d ).r * 100;\n"
#endif
    "  m.x += sin( fGlobalTime ) * 0.1;\n"
    "  m.y += fGlobalTime * 0.25;\n"
    "\n"
    "  vec4 t = plas( m * 3.14, fGlobalTime ) / d;\n"
    "  t = clamp( t, 0.0, 1.0 );\n"
    "  out_color = f + t;\n"
#ifdef __APPLE__
    "  gl_FragColor = out_color;\n"
#endif
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
      printf("[Renderer] SDL_Init failed\n");
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
      printf("[Renderer] SDL_SetVideoMode failed\n");
      SDL_Quit();
      return false;
    }

    SDL_EnableUNICODE(true);
    SDL_EnableKeyRepeat(250, 20);

    //if (settings->bVsync)
    //wglSwapIntervalEXT(1);

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
      else if (E.type == SDL_MOUSEMOTION)
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_MOVE;
        mouseEventBuffer[mouseEventBufferCount].x = E.motion.x;
        mouseEventBuffer[mouseEventBufferCount].y = E.motion.y;
        if (E.motion.state & SDL_BUTTON(1)) mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT;
        mouseEventBufferCount++;
      }
      else if (E.type == SDL_MOUSEBUTTONDOWN)
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_DOWN;
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
      else if (E.type == SDL_MOUSEBUTTONUP)
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_UP;
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

  // Changed the two functions below to support OSX a little better

  void SetShaderConstant( char * szConstName, float x ) {
    
    glUseProgram(theShader);

    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
         glUniform1f( location, x );
    }
 }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
    glUseProgram(theShader);
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
     glUniform2f(location, x, y);
     //glProgramUniform2fEXT( theShader, location, x, y );
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

    glUseProgram(theShader);
    GLint location = glGetUniformLocation( theShader, szTextureName );

    if ( location != -1 )
    {
      glUniform1i( location, ((GLTexture*)tex)->unit );
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

  void RenderQuad( const Vertex & a, const Vertex & b, const Vertex & c, const Vertex & d )
  {
    glBegin(GL_QUADS);
    glColor4ubv( (GLubyte*)&a.c ); glTexCoord2f( a.u, a.v ); glVertex2f( a.x, a.y );
    glColor4ubv( (GLubyte*)&b.c ); glTexCoord2f( b.u, b.v ); glVertex2f( b.x, b.y );
    glColor4ubv( (GLubyte*)&c.c ); glTexCoord2f( c.u, c.v ); glVertex2f( c.x, c.y );
    glColor4ubv( (GLubyte*)&d.c ); glTexCoord2f( d.u, d.v ); glVertex2f( d.x, d.y );
    glEnd();
  }

  void RenderLine( const Vertex & a, const Vertex & b )
  {
    glBegin(GL_LINES);
    glColor4ubv( (GLubyte*)&a.c ); glTexCoord2f( a.u, a.v ); glVertex2f( a.x, a.y );
    glColor4ubv( (GLubyte*)&b.c ); glTexCoord2f( b.u, b.v ); glVertex2f( b.x, b.y );
    glEnd();
  }

}
