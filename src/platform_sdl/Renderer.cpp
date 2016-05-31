#ifdef _WIN32
#include <windows.h>
#endif
#include <SDL.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#ifdef _WIN32
#include <GL/wGLew.h>
#endif
#include "../Renderer.h"
#include <string.h>

#include "UniConversion.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Scintilla.h"

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
    "#version 410 core\n"
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
    "layout(location = 0) out vec4 out_color; // out_color must be written in order to see anything\n"
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
    "  m.y = 1 / length(uv) * .2;\n"
    "  float d = m.y;\n"
    "\n"
    "  float f = texture( texFFT, d ).r * 100;\n"
    "  m.x += sin( fGlobalTime ) * 0.1;\n"
    "  m.y += fGlobalTime * 0.25;\n"
    "\n"
    "  vec4 t = plas( m * 3.14, fGlobalTime ) / d;\n"
    "  t = clamp( t, 0.0, 1.0 );\n"
    "  out_color = f + t;\n"
    "}";

  SDL_Window * mWindow = NULL;
  bool run = true;

  GLuint theShader = NULL;
  GLuint glhVertexShader = NULL;
  GLuint glhFullscreenQuadVB = NULL;
  GLuint glhFullscreenQuadVA = NULL;
  GLuint glhGUIVB = NULL;
  GLuint glhGUIVA = NULL;
  GLuint glhGUIProgram = NULL;

  int nWidth = 0;
  int nHeight = 0;

  void MatrixOrthoOffCenterLH(float * pout, float l, float r, float b, float t, float zn, float zf)
  {
    memset( pout, 0, sizeof(float) * 4 * 4 );
    pout[0 + 0 * 4] = 2.0f / (r - l);
    pout[1 + 1 * 4] = 2.0f / (t - b);
    pout[2 + 2 * 4] = 1.0f / (zf -zn);
    pout[3 + 0 * 4] = -1.0f -2.0f *l / (r - l);
    pout[3 + 1 * 4] = 1.0f + 2.0f * t / (b - t);
    pout[3 + 2 * 4] = zn / (zn -zf);
    pout[3 + 3 * 4] = 1.0;
  }

  int readIndex = 0;
  int writeIndex = 1;
  GLuint pbo[2];

  bool Open( RENDERER_SETTINGS * settings )
  {
    theShader = NULL;
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0)
    {
      printf("[Renderer] SDL_Init failed: %s\n",SDL_GetError());
      return false;
    }

    uint32_t flags = SDL_WINDOW_OPENGL;
    if (settings->windowMode == RENDERER_WINDOWMODE_FULLSCREEN)
      flags |= SDL_WINDOW_FULLSCREEN;

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, true );

    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

    nWidth = settings->nWidth;
    nHeight = settings->nHeight;

    mWindow = SDL_CreateWindow("BONZOMATIC - SDL edition",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      settings->nWidth, settings->nHeight,
      flags);
    if (!mWindow)
    {
      printf("[SDL] SDL_CreateWindow failed: %s\n",SDL_GetError());
      return false;
    }

    if (!SDL_GL_CreateContext(mWindow))
    {
      printf("[SDL] SDL_GL_CreateContext failed: %s\n",SDL_GetError());
      return false;
    }

    SDL_StartTextInput();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
      printf("[SDL] glewInit failed\n");
      return false;
    }
    GLenum i = glGetError(); // reset glew error

#ifdef _WIN32
    if (settings->bVsync)
      wglSwapIntervalEXT(1);
#endif

    static float pFullscreenQuadVertices[] =
    {
      -1.0, -1.0,  0.5, 0.0, 0.0,
      -1.0,  1.0,  0.5, 0.0, 1.0,
       1.0, -1.0,  0.5, 1.0, 0.0,
       1.0,  1.0,  0.5, 1.0, 1.0,
    };

    glGenBuffers( 1, &glhFullscreenQuadVB );
    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 5 * 4, pFullscreenQuadVertices, GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, NULL );

    glGenVertexArrays(1, &glhFullscreenQuadVA);

    glhVertexShader = glCreateShader( GL_VERTEX_SHADER );

    char * szVertexShader =
      "#version 410 core\n"
      "in vec3 in_pos;\n"
      "in vec2 in_texcoord;\n"
      "out vec2 out_texcoord;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = vec4( in_pos.x, in_pos.y, in_pos.z, 1.0 );\n"
      "  out_texcoord = in_texcoord;\n"
      "}";
    GLint nShaderSize = strlen(szVertexShader);

    glShaderSource(glhVertexShader, 1, (const GLchar**)&szVertexShader, &nShaderSize);
    glCompileShader(glhVertexShader);

    GLint size = 0;
    GLint result = 0;
    char szErrorBuffer[5000];
    glGetShaderInfoLog(glhVertexShader, 4000, &size, szErrorBuffer);
    glGetShaderiv(glhVertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
      printf("[Renderer] Vertex shader compilation failed\n");
      return false;
    }

#define GUIQUADVB_SIZE (1024 * 6)

    char * defaultGUIVertexShader =
      "#version 410 core\n"
      "in vec3 in_pos;\n"
      "in vec4 in_color;\n"
      "in vec2 in_texcoord;\n"
      "in float in_factor;\n"
      "out vec4 out_color;\n"
      "out vec2 out_texcoord;\n"
      "out float out_factor;\n"
      "uniform vec2 v2Offset;\n"
      "uniform mat4 matProj;\n"
      "void main()\n"
      "{\n"
      "  vec4 pos = vec4( in_pos + vec3(v2Offset,0), 1.0 );\n"
      "  gl_Position = pos * matProj;\n"
      "  out_color = in_color;\n"
      "  out_texcoord = in_texcoord;\n"
      "  out_factor = in_factor;\n"
      "}\n";
    char * defaultGUIPixelShader =
      "#version 410 core\n"
      "uniform sampler2D tex;\n"
      "in vec4 out_color;\n"
      "in vec2 out_texcoord;\n"
      "in float out_factor;\n"
      "out vec4 frag_color;\n"
      "void main()\n"
      "{\n"
      "  vec4 v4Texture = out_color * texture( tex, out_texcoord );\n"
      "  vec4 v4Color = out_color;\n"
      "  frag_color = mix( v4Texture, v4Color, out_factor );\n"
      "}\n";

    glhGUIProgram = glCreateProgram();

    GLuint vshd = glCreateShader(GL_VERTEX_SHADER);
    nShaderSize = strlen(defaultGUIVertexShader);

    glShaderSource(vshd, 1, (const GLchar**)&defaultGUIVertexShader, &nShaderSize);
    glCompileShader(vshd);
    glGetShaderInfoLog(vshd, 4000, &size, szErrorBuffer);
    glGetShaderiv(vshd, GL_COMPILE_STATUS, &result);
    if (!result)
    {
      printf("[Renderer] Default GUI vertex shader compilation failed\n");
      return false;
    }

    GLuint fshd = glCreateShader(GL_FRAGMENT_SHADER);
    nShaderSize = strlen(defaultGUIPixelShader);

    glShaderSource(fshd, 1, (const GLchar**)&defaultGUIPixelShader, &nShaderSize);
    glCompileShader(fshd);
    glGetShaderInfoLog(fshd, 4000, &size, szErrorBuffer);
    glGetShaderiv(fshd, GL_COMPILE_STATUS, &result);
    if (!result)
    {
      printf("[Renderer] Default GUI pixel shader compilation failed\n");
      return false;
    }

    glAttachShader(glhGUIProgram, vshd);
    glAttachShader(glhGUIProgram, fshd);
    glLinkProgram(glhGUIProgram);
    glGetProgramiv(glhGUIProgram, GL_LINK_STATUS, &result);
    if (!result)
    {
      return false;
    }

    glGenBuffers( 1, &glhGUIVB );
    glBindBuffer( GL_ARRAY_BUFFER, glhGUIVB );

    glGenVertexArrays(1, &glhGUIVA);

    //create PBOs to hold the data. this allocates memory for them too
    glGenBuffers(2, pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, settings->nWidth * settings->nHeight * sizeof(unsigned int), NULL, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, settings->nWidth * settings->nHeight * sizeof(unsigned int), NULL, GL_STREAM_READ);
    //unbind buffers for now
    glBindBuffer(GL_PIXEL_PACK_BUFFER, NULL);

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
      else if (E.type == SDL_TEXTINPUT)
      {
        keyEventBuffer[keyEventBufferCount].ctrl  = false;
        keyEventBuffer[keyEventBufferCount].alt   = false;
        keyEventBuffer[keyEventBufferCount].shift = false;
        keyEventBuffer[keyEventBufferCount].scanCode = 0;

        unsigned int c = E.text.text[0];
        unsigned int charLength = Scintilla::UTF8CharLength(E.text.text[0]);
        if (charLength > 1)
        {
          c = 0;
          Scintilla::UTF16FromUTF8( E.text.text, charLength, (wchar_t*)&c, sizeof(unsigned int) );
        }

        keyEventBuffer[keyEventBufferCount].character = c;
        keyEventBufferCount++;
      }
      else if (E.type == SDL_KEYDOWN)
      {
        if (E.key.keysym.sym == SDLK_F4 && (E.key.keysym.mod == KMOD_LALT || E.key.keysym.mod == KMOD_RALT))
        {
          run = false;
        }
        int sciKey = 0;
        bool bNormalKey = false;
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
//           case SDLK_LSUPER:       sciKey = SCK_WIN;       break;
//           case SDLK_RSUPER:       sciKey = SCK_RWIN;      break;
          case SDLK_MENU:         sciKey = SCK_MENU;      break;
//           case SDLK_SLASH:        sciKey = '/';           break;
//           case SDLK_ASTERISK:     sciKey = '`';           break;
//           case SDLK_LEFTBRACKET:  sciKey = '[';           break;
//           case SDLK_BACKSLASH:    sciKey = '\\';          break;
//           case SDLK_RIGHTBRACKET: sciKey = ']';           break;
          case SDLK_F1:           sciKey = 282;           break;
          case SDLK_F2:           sciKey = 283;           break;
          case SDLK_F3:           sciKey = 284;           break;
          case SDLK_F4:           sciKey = 285;           break;
          case SDLK_F5:           sciKey = 286;           break;
          case SDLK_F6:           sciKey = 287;           break;
          case SDLK_F7:           sciKey = 288;           break;
          case SDLK_F8:           sciKey = 289;           break;
          case SDLK_F9:           sciKey = 290;           break;
          case SDLK_F10:          sciKey = 291;           break;
          case SDLK_F11:          sciKey = 292;           break;
          case SDLK_F12:          sciKey = 293;           break;
          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
          case SDLK_LALT:
          case SDLK_RALT:
          case SDLK_LCTRL:
          case SDLK_RCTRL:
          case SDLK_LGUI:
          case SDLK_RGUI:
            sciKey = 0;
            break;
          default:
            bNormalKey = true;
            sciKey = E.key.keysym.sym;
        }

        if ((bNormalKey && E.key.keysym.mod) || !bNormalKey)
        {
          keyEventBuffer[keyEventBufferCount].ctrl  = E.key.keysym.mod & KMOD_LCTRL  || E.key.keysym.mod & KMOD_RCTRL || E.key.keysym.mod & KMOD_LGUI || E.key.keysym.mod & KMOD_RGUI;
          keyEventBuffer[keyEventBufferCount].alt   = E.key.keysym.mod & KMOD_LALT   || E.key.keysym.mod & KMOD_RALT;
          keyEventBuffer[keyEventBufferCount].shift = E.key.keysym.mod & KMOD_LSHIFT || E.key.keysym.mod & KMOD_RSHIFT;
          keyEventBuffer[keyEventBufferCount].scanCode = sciKey;
          keyEventBuffer[keyEventBufferCount].character = 0;
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
  }
  void EndFrame()
  {
    SDL_GL_SwapWindow(mWindow);
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
    glBindVertexArray(glhFullscreenQuadVA);

    glUseProgram(theShader);

    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );

    GLuint position = glGetAttribLocation( theShader, "in_pos" );
    glVertexAttribPointer( position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(0 * sizeof(GLfloat)) );

    GLuint texcoord = glGetAttribLocation( theShader, "in_texcoord" );
    glVertexAttribPointer( texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(3 * sizeof(GLfloat)) );

    glEnableVertexAttribArray( position );
    glEnableVertexAttribArray( texcoord );
    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    glDisableVertexAttribArray( texcoord );
    glDisableVertexAttribArray( position );

    glUseProgram(NULL);
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

    glAttachShader(prg, glhVertexShader);
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

    if (theShader)
      glDeleteProgram(theShader);

    theShader = prg;

    return true;
  }

  void SetShaderConstant( char * szConstName, float x )
  {
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
      glProgramUniform1f( theShader, location, x );
    }
  }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
      glProgramUniform2f( theShader, location, x, y );
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
      glProgramUniform1i( theShader, location, ((GLTexture*)tex)->unit );
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

  Texture * CreateA8TextureFromData( int w, int h, unsigned char * data )
  {
    GLuint glTexId = 0;
    glGenTextures(1, &glTexId);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    unsigned int * p32bitData = new unsigned int[ w * h ];
    for(int i=0; i<w*h; i++) p32bitData[i] = (data[i] << 24) | 0xFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, p32bitData);
    delete[] p32bitData;

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

  //////////////////////////////////////////////////////////////////////////
  // text rendering

  int nDrawCallCount = 0;
  Texture * lastTexture = NULL;
  void StartTextRendering()
  {
    glUseProgram(glhGUIProgram);
    glBindVertexArray(glhGUIVA);

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  int bufferPointer = 0;
  unsigned char buffer[GUIQUADVB_SIZE * sizeof(float) * 7];
  bool lastModeIsQuad = true;
  void __FlushRenderCache()
  {
    if (!bufferPointer) return;

    glBindBuffer( GL_ARRAY_BUFFER, glhGUIVB );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 7 * bufferPointer, buffer, GL_DYNAMIC_DRAW );

    GLuint position = glGetAttribLocation( glhGUIProgram, "in_pos" );
    glVertexAttribPointer( position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(0 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( position );

    GLuint color = glGetAttribLocation( glhGUIProgram, "in_color" );
    glVertexAttribPointer( color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(float) * 7, (GLvoid*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( color );

    GLuint texcoord = glGetAttribLocation( glhGUIProgram, "in_texcoord" );
    glVertexAttribPointer( texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(4 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( texcoord );

    GLuint factor = glGetAttribLocation( glhGUIProgram, "in_factor" );
    glVertexAttribPointer( factor, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(6 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( factor );

    if (lastModeIsQuad)
    {
      glDrawArrays( GL_TRIANGLES, 0, bufferPointer );
    }
    else
    {
      glDrawArrays( GL_LINES, 0, bufferPointer );
    }

    bufferPointer = 0;
  }
  void __WriteVertexToBuffer( const Vertex & v )
  {
    if (bufferPointer >= GUIQUADVB_SIZE)
    {
      __FlushRenderCache();
    }

    float * f = (float*)(buffer + bufferPointer * sizeof(float) * 7);
    *(f++) = v.x;
    *(f++) = v.y;
    *(f++) = 0.0;
    *(unsigned int *)(f++) = v.c;
    *(f++) = v.u;
    *(f++) = v.v;
    *(f++) = lastTexture ? 0.0f : 1.0f;
    bufferPointer++;
  }
  void BindTexture( Texture * tex )
  {
    if (lastTexture != tex)
    {
      lastTexture = tex;
      if (tex)
      {
        __FlushRenderCache();

        GLint location = glGetUniformLocation( glhGUIProgram, "tex" );
        if ( location != -1 )
        {
          glProgramUniform1i( glhGUIProgram, location, ((GLTexture*)tex)->unit );
          glActiveTexture( GL_TEXTURE0 + ((GLTexture*)tex)->unit );
          switch( tex->type)
          {
            case TEXTURETYPE_1D: glBindTexture( GL_TEXTURE_1D, ((GLTexture*)tex)->ID ); break;
            case TEXTURETYPE_2D: glBindTexture( GL_TEXTURE_2D, ((GLTexture*)tex)->ID ); break;
          }
        }

      }
    }
  }

  void RenderQuad( const Vertex & a, const Vertex & b, const Vertex & c, const Vertex & d )
  {
    if (!lastModeIsQuad)
    {
      __FlushRenderCache();
      lastModeIsQuad = true;
    }
    __WriteVertexToBuffer(a);
    __WriteVertexToBuffer(b);
    __WriteVertexToBuffer(d);
    __WriteVertexToBuffer(b);
    __WriteVertexToBuffer(c);
    __WriteVertexToBuffer(d);
  }

  void RenderLine( const Vertex & a, const Vertex & b )
  {
    if (lastModeIsQuad)
    {
      __FlushRenderCache();
      lastModeIsQuad = false;
    }
    __WriteVertexToBuffer(a);
    __WriteVertexToBuffer(b);
  }

  void SetTextRenderingViewport( Scintilla::PRectangle rect )
  {
    __FlushRenderCache();

    float pGUIMatrix[16];
    MatrixOrthoOffCenterLH( pGUIMatrix, 0.0f, (float)nWidth, (float)nHeight, 0.0f, -1.0f, 1.0f );

    GLint location = glGetUniformLocation( glhGUIProgram, "matProj" );
    if ( location != -1 )
    {
       glProgramUniformMatrix4fv( glhGUIProgram, location, 1, GL_FALSE, pGUIMatrix );
    }

    location = glGetUniformLocation( glhGUIProgram, "v2Offset" );
    if ( location != -1 )
    {
      glProgramUniform2f( glhGUIProgram, location, rect.left, rect.top );
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.left, nHeight - rect.bottom, rect.right - rect.left, rect.bottom - rect.top);
  }
  void EndTextRendering()
  {
    __FlushRenderCache();

    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    GLuint position = glGetAttribLocation( glhGUIProgram, "in_pos" );
    GLuint color = glGetAttribLocation( glhGUIProgram, "in_color" );
    GLuint texcoord = glGetAttribLocation( glhGUIProgram, "in_texcoord" );
    GLuint factor = glGetAttribLocation( glhGUIProgram, "in_factor" );

    glDisableVertexAttribArray( factor );
    glDisableVertexAttribArray( texcoord );
    glDisableVertexAttribArray( color );
    glDisableVertexAttribArray( position );

    glUseProgram(NULL);

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
  }

  //////////////////////////////////////////////////////////////////////////

  bool GrabFrame( void * pPixelBuffer )
  {
    writeIndex = (writeIndex + 1) % 2;
    readIndex = (readIndex + 1) % 2;

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[writeIndex]);
    glReadPixels(0, 0, nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[readIndex]);
    unsigned char * downsampleData = (unsigned char *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (downsampleData)
    {
      unsigned char * src = downsampleData;
      unsigned char * dst = (unsigned char*)pPixelBuffer + nWidth * (nHeight - 1) * sizeof(unsigned int);
      for (int i=0; i<nHeight; i++)
      {
        memcpy( dst, src, sizeof(unsigned int) * nWidth );
        src += sizeof(unsigned int) * nWidth;
        dst -= sizeof(unsigned int) * nWidth;
      }
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, NULL);

    return true;
  }

}
