
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

#define GLEW_NO_GLU
#include "GL/glew.h"
#ifdef _WIN32
#include <GL/wGLew.h>
#endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "../Renderer.h"
#include <string.h>

#include "UniConversion.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Scintilla.h"

#ifdef __APPLE__
#include "../TouchBar.h"
#endif

const char * shaderKeyword =
  "discard struct if else switch case default break goto return for while do continue";

const char * shaderType =
  "attribute const in inout out uniform varying invariant "
  "centroid flat smooth noperspective layout patch sample "
  "subroutine lowp mediump highp precision "
  "void float vec2 vec3 vec4 bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 "
  "mat2 mat3 mat4 int bool "
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
  const char * defaultShaderFilename = "shader.glsl";
  const char defaultShader[65536] =
    "#version 410 core\n"
    "\n"
    "uniform float fGlobalTime; // in seconds\n"
    "uniform vec2 v2Resolution; // viewport resolution (in pixels)\n"
    "uniform float fFrameTime; // duration of the last frame, in seconds\n"
    "\n"
    "uniform sampler1D texFFT; // towards 0.0 is bass / lower freq, towards 1.0 is higher / treble freq\n"
    "uniform sampler1D texFFTSmoothed; // this one has longer falloff and less harsh transients\n"
    "uniform sampler1D texFFTIntegrated; // this is continually increasing\n"
    "uniform sampler2D texPreviousFrame; // screenshot of the previous frame\n"
    "{%textures:begin%}" // leave off \n here
    "uniform sampler2D {%textures:name%};\n"
    "{%textures:end%}" // leave off \n here
    "{%midi:begin%}" // leave off \n here
    "uniform float {%midi:name%};\n"
    "{%midi:end%}" // leave off \n here
    "\n"
    "layout(location = 0) out vec4 out_color; // out_color must be written in order to see anything\n"
    "\n"
    "vec4 plas( vec2 v, float time )\n"
    "{\n"
    "\tfloat c = 0.5 + sin( v.x * 10.0 ) + cos( sin( time + v.y ) * 20.0 );\n"
    "\treturn vec4( sin(c * 0.2 + cos(time)), c * 0.15, cos( c * 0.1 + time / .4 ) * .25, 1.0 );\n"
    "}\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "\tvec2 uv = vec2(gl_FragCoord.x / v2Resolution.x, gl_FragCoord.y / v2Resolution.y);\n"
    "\tuv -= 0.5;\n"
    "\tuv /= vec2(v2Resolution.y / v2Resolution.x, 1);\n"
    "\n"
    "\tvec2 m;\n"
    "\tm.x = atan(uv.x / uv.y) / 3.14;\n"
    "\tm.y = 1 / length(uv) * .2;\n"
    "\tfloat d = m.y;\n"
    "\n"
    "\tfloat f = texture( texFFT, d ).r * 100;\n"
    "\tm.x += sin( fGlobalTime ) * 0.1;\n"
    "\tm.y += fGlobalTime * 0.25;\n"
    "\n"
    "\tvec4 t = plas( m * 3.14, fGlobalTime ) / d;\n"
    "\tt = clamp( t, 0.0, 1.0 );\n"
    "\tout_color = f + t;\n"
    "}";

  GLFWwindow * mWindow = NULL;
  bool run = true;

  GLuint theShader = 0;
  GLuint glhShaderFB = 0;
  GLuint glhShaderTex = 0;
  GLuint glhVertexShader = 0;
  GLuint glhFullscreenQuadVB = 0;
  GLuint glhFullscreenQuadVA = 0;
  GLuint glhGUIVB = 0;
  GLuint glhGUIVA = 0;
  GLuint glhGUIProgram = 0;

  int nWidth = 0;
  int nHeight = 0;
  float fScale = 1.0f;
  bool bLinearFilter = false;

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

  static void error_callback(int error, const char *description) {
    switch (error) {
    case GLFW_API_UNAVAILABLE:
      printf("OpenGL is unavailable: ");
      break;
    case GLFW_VERSION_UNAVAILABLE:
      printf("OpenGL 4.1 (the minimum requirement) is not available: ");
      break;
    }
    printf("%s\n", description);
  }
  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void character_callback(GLFWwindow* window, unsigned int codepoint);
  void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
  void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

  bool Open( RENDERER_SETTINGS * settings )
  {
    glfwSetErrorCallback(error_callback);
    theShader = 0;
    
#ifdef __APPLE__
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if(!glfwInit())
    {
      printf("[Renderer] GLFW init failed\n");
      return false;
    }
    printf("[GLFW] Version String: %s\n", glfwGetVersionString());

    nWidth = settings->nWidth;
    nHeight = settings->nHeight;
    fScale = settings->fScale;
    bLinearFilter = settings->bLinearFilter;

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE);
#endif

    // TODO: change in case of resize support
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Prevent fullscreen window minimize on focus loss
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

    GLFWmonitor *monitor = settings->windowMode == RENDERER_WINDOWMODE_FULLSCREEN ? glfwGetPrimaryMonitor() : NULL;

    mWindow = glfwCreateWindow(nWidth, nHeight, "BONZOMATIC - GLFW edition", monitor, NULL);
    if (!mWindow)
    {
      printf("[GLFW] Window creation failed\n");
      glfwTerminate();
      return false;
    }

#ifdef __APPLE__
#ifdef BONZOMATIC_ENABLE_TOUCHBAR
    ShowTouchBar(mWindow);
#endif
#endif
      
    glfwMakeContextCurrent(mWindow);

    // TODO: here add text callbacks
    glfwSetKeyCallback(mWindow, key_callback);
    glfwSetCharCallback(mWindow, character_callback);
    glfwSetCursorPosCallback(mWindow, cursor_position_callback);
    glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        printf("[GLFW] glewInit failed: %s\n", glewGetErrorString(err));
        glfwTerminate();
        return false;
    }
    printf("[GLFW] Using GLEW %s\n", glewGetString(GLEW_VERSION));
    glGetError(); // reset glew error

    glfwSwapInterval(1);

#ifdef _WIN32
    if (settings->bVsync)
      wglSwapIntervalEXT(1);
#endif

    printf("[GLFW] OpenGL Version %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Now, since OpenGL is behaving a lot in fullscreen modes, lets collect the real obtained size!
    printf("[GLFW] Requested framebuffer size: %d x %d\n", nWidth, nHeight);
    int fbWidth = 1;
    int fbHeight = 1;
    glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);
    nWidth = settings->nWidth = fbWidth;
    nHeight = settings->nHeight = fbHeight;
    printf("[GLFW] Obtained framebuffer size: %d x %d\n", fbWidth, fbHeight);

    if (fScale != 1.0f) {
      glGenTextures( 1, &glhShaderTex );
      glBindTexture( GL_TEXTURE_2D, glhShaderTex );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, nWidth*fScale, nHeight*fScale, 0, GL_RGBA, GL_FLOAT, NULL );
      glBindTexture( GL_TEXTURE_2D, 0 );

      glGenFramebuffers( 1, &glhShaderFB );
      glBindFramebuffer( GL_FRAMEBUFFER, glhShaderFB );
      glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glhShaderTex, 0 );
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    }
    
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
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glGenVertexArrays(1, &glhFullscreenQuadVA);

    glhVertexShader = glCreateShader( GL_VERTEX_SHADER );

    const char * szVertexShader =
      "#version 410 core\n"
      "in vec3 in_pos;\n"
      "in vec2 in_texcoord;\n"
      "out vec2 out_texcoord;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = vec4( in_pos.x, in_pos.y, in_pos.z, 1.0 );\n"
      "  out_texcoord = in_texcoord;\n"
      "}";
    GLint nShaderSize = (GLint)strlen(szVertexShader);

    glShaderSource(glhVertexShader, 1, (const GLchar**)&szVertexShader, &nShaderSize);
    glCompileShader(glhVertexShader);

    GLint size = 0;
    GLint result = 0;
    char szErrorBuffer[5000];
    glGetShaderInfoLog(glhVertexShader, 4000, &size, szErrorBuffer);
    glGetShaderiv(glhVertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
      printf("[Renderer] Vertex shader compilation failed\n%s\n", szErrorBuffer);
      return false;
    }

#define GUIQUADVB_SIZE (1024 * 6)

    const char * defaultGUIVertexShader =
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
    const char * defaultGUIPixelShader =
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
    nShaderSize = (GLint)strlen(defaultGUIVertexShader);

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
    nShaderSize = (GLint)strlen(defaultGUIPixelShader);

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
    glBufferData(GL_PIXEL_PACK_BUFFER, nWidth * nHeight * sizeof(unsigned int), NULL, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, nWidth * nHeight * sizeof(unsigned int), NULL, GL_STREAM_READ);
    //unbind buffers for now
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glViewport(0, 0, nWidth, nHeight);
    
    run = true;

    return true;
  }

  KeyEvent keyEventBuffer[512];
  int keyEventBufferCount = 0;
  MouseEvent mouseEventBuffer[512];
  int mouseEventBufferCount = 0;
  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      if ((key==GLFW_KEY_F4 && (mods&GLFW_MOD_ALT)) || (key==GLFW_KEY_ESCAPE&&(mods&GLFW_MOD_SHIFT))) {
        run = false;
      }
      int sciKey = 0;
      bool bNormalKey = false;
      switch (key) {
        case GLFW_KEY_DOWN:         sciKey = SCK_DOWN;      break;
        case GLFW_KEY_UP:           sciKey = SCK_UP;        break;
        case GLFW_KEY_LEFT:         sciKey = SCK_LEFT;      break;
        case GLFW_KEY_RIGHT:        sciKey = SCK_RIGHT;     break;
        case GLFW_KEY_HOME:         sciKey = SCK_HOME;      break;
        case GLFW_KEY_END:          sciKey = SCK_END;       break;
        case GLFW_KEY_PAGE_UP:      sciKey = SCK_PRIOR;     break;
        case GLFW_KEY_PAGE_DOWN:    sciKey = SCK_NEXT;      break;
        case GLFW_KEY_DELETE:       sciKey = SCK_DELETE;    break;
        case GLFW_KEY_INSERT:       sciKey = SCK_INSERT;    break;
        case GLFW_KEY_ESCAPE:       sciKey = SCK_ESCAPE;    break;
        case GLFW_KEY_BACKSPACE:    sciKey = SCK_BACK;      break;
        case GLFW_KEY_TAB:          sciKey = SCK_TAB;       break;
        case GLFW_KEY_ENTER:        sciKey = SCK_RETURN;    break;
        case GLFW_KEY_KP_ADD:       sciKey = SCK_ADD;       break;
        case GLFW_KEY_KP_SUBTRACT:  sciKey = SCK_SUBTRACT;  break;
        case GLFW_KEY_KP_DIVIDE:    sciKey = SCK_DIVIDE;    break;
//           case GLFW_KEY_LSUPER:       sciKey = SCK_WIN;       break;
//           case GLFW_KEY_RSUPER:       sciKey = SCK_RWIN;      break;
        case GLFW_KEY_MENU:         sciKey = SCK_MENU;      break;
//           case GLFW_KEY_SLASH:        sciKey = '/';           break;
//           case GLFW_KEY_ASTERISK:     sciKey = '`';           break;
//           case GLFW_KEY_LEFTBRACKET:  sciKey = '[';           break;
//           case GLFW_KEY_BACKSLASH:    sciKey = '\\';          break;
//           case GLFW_KEY_RIGHTBRACKET: sciKey = ']';           break;
        case GLFW_KEY_F1:           sciKey = 282;           break;
        case GLFW_KEY_F2:           sciKey = 283;           break;
        case GLFW_KEY_F3:           sciKey = 284;           break;
        case GLFW_KEY_F4:           sciKey = 285;           break;
        case GLFW_KEY_F5:           sciKey = 286;           break;
        case GLFW_KEY_F6:           sciKey = 287;           break;
        case GLFW_KEY_F7:           sciKey = 288;           break;
        case GLFW_KEY_F8:           sciKey = 289;           break;
        case GLFW_KEY_F9:           sciKey = 290;           break;
        case GLFW_KEY_F10:          sciKey = 291;           break;
        case GLFW_KEY_F11:          sciKey = 292;           break;
        case GLFW_KEY_F12:          sciKey = 293;           break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_RIGHT_ALT:
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_LEFT_SUPER:
        case GLFW_KEY_RIGHT_SUPER:
          sciKey = 0;
          break;
        default:
          bNormalKey = true;
          // TODO: Horrible hack to migrate from GLFW (that uses ascii maj for keys) to scintilla min keys
          if ( (key >= GLFW_KEY_A) && (key <= GLFW_KEY_Z) ) {
            sciKey = key+32;
          }
          else {
            sciKey = 0;
          }
      }
      if ((bNormalKey && mods) || !bNormalKey)
      {
        keyEventBuffer[keyEventBufferCount].ctrl  = (mods & GLFW_MOD_CONTROL) || (mods & GLFW_MOD_SUPER);
        keyEventBuffer[keyEventBufferCount].alt   = mods & GLFW_MOD_ALT;
        keyEventBuffer[keyEventBufferCount].shift = mods & GLFW_MOD_SHIFT;
        keyEventBuffer[keyEventBufferCount].scanCode = sciKey;
        keyEventBuffer[keyEventBufferCount].character = 0;
        keyEventBufferCount++;
      }
    }
  }
  void character_callback(GLFWwindow* window, unsigned int codepoint)
  {
    keyEventBuffer[keyEventBufferCount].ctrl  = false;
    keyEventBuffer[keyEventBufferCount].alt   = false;
    keyEventBuffer[keyEventBufferCount].shift = false;
    keyEventBuffer[keyEventBufferCount].scanCode = 0;
    // TODO: handle special things with Scintilla for UFT8 codepoints?
    keyEventBuffer[keyEventBufferCount].character = codepoint;
    keyEventBufferCount++;
  }
  void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
  {
    mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_MOVE;
    mouseEventBuffer[mouseEventBufferCount].x = xpos;
    mouseEventBuffer[mouseEventBufferCount].y = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT;
    mouseEventBufferCount++;
  }
  void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
  {
    if (action == GLFW_PRESS) {
      mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_DOWN;
    }
    else if (action == GLFW_RELEASE) {
      mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_UP;
    }
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseEventBuffer[mouseEventBufferCount].x = xpos;
    mouseEventBuffer[mouseEventBufferCount].y = ypos;
    switch(button)
    {
      case GLFW_MOUSE_BUTTON_MIDDLE: mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_MIDDLE; break;
      case GLFW_MOUSE_BUTTON_RIGHT:  mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_RIGHT; break;
      case GLFW_MOUSE_BUTTON_LEFT:
      default:                mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT; break;
    }
    mouseEventBufferCount++;
  }
  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
  {
    mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_SCROLL;
    mouseEventBuffer[mouseEventBufferCount].x = xoffset;
    mouseEventBuffer[mouseEventBufferCount].y = yoffset;
    mouseEventBufferCount++;
  }

  void StartFrame()
  {
    glClearColor(0.08f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
  void EndFrame()
  {
    keyEventBufferCount = 0;
    mouseEventBufferCount = 0;
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }
  bool WantsToQuit()
  {
    return glfwWindowShouldClose(mWindow) || !run;
  }
  void Close()
  {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  void RenderFullscreenQuad()
  {
    glBindVertexArray(glhFullscreenQuadVA);

    if (fScale != 1.0f) {
      glBindFramebuffer(GL_FRAMEBUFFER, glhShaderFB);
    }

    glUseProgram(theShader);

    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );

    const GLint position = glGetAttribLocation( theShader, "in_pos" );
    if (position >= 0)
    {
      glVertexAttribPointer( position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(0 * sizeof(GLfloat)) );
      glEnableVertexAttribArray( position );
    }

    const GLint texcoord = glGetAttribLocation( theShader, "in_texcoord" );
    if (texcoord >= 0)
    {
      glVertexAttribPointer( texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(3 * sizeof(GLfloat)) );
      glEnableVertexAttribArray( texcoord );
    }

    glBindBuffer( GL_ARRAY_BUFFER, glhFullscreenQuadVB );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    if (texcoord >= 0)
      glDisableVertexAttribArray( texcoord );

    if (position >= 0)
      glDisableVertexAttribArray( position );

    glUseProgram(0);

    if (fScale != 1.0f) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, glhShaderFB);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      glBlitFramebuffer(0, 0, nWidth*fScale, nHeight*fScale, 0, 0, nWidth, nHeight, GL_COLOR_BUFFER_BIT, bLinearFilter ? GL_LINEAR : GL_NEAREST);
    }
  }

  bool ReloadShader( const char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
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

  void SetShaderConstant( const char * szConstName, float x )
  {
    GLint location = glGetUniformLocation( theShader, szConstName );
    if ( location != -1 )
    {
      glProgramUniform1f( theShader, location, x );
    }
  }

  void SetShaderConstant( const char * szConstName, float x, float y )
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

  Texture * CreateRGBA8Texture()
  {
    void * data = NULL;
    GLenum internalFormat = GL_SRGB8_ALPHA8;
    GLenum srcFormat = GL_FLOAT;
    GLenum format = GL_UNSIGNED_BYTE;

    GLuint glTexId = 0;
    glGenTextures( 1, &glTexId );
    glBindTexture( GL_TEXTURE_2D, glTexId );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    GLTexture * tex = new GLTexture();
    tex->width = nWidth;
    tex->height = nHeight;
    tex->ID = glTexId;
    tex->type = TEXTURETYPE_2D;
    tex->unit = textureUnit++;
    return tex;
  }

  Texture * CreateRGBA8TextureFromFile( const char * szFilename )
  {
    int comp = 0;
    int width = 0;
    int height = 0;
    void * data = NULL;
    GLenum internalFormat = GL_SRGB8_ALPHA8;
    GLenum srcFormat = GL_RGBA;
    GLenum format = GL_UNSIGNED_BYTE;
    if ( stbi_is_hdr( szFilename ) )
    {
      internalFormat = GL_RGBA32F;
      format = GL_FLOAT;
      data = stbi_loadf( szFilename, &width, &height, &comp, STBI_rgb_alpha );
    }
    else
    {
      data = stbi_load( szFilename, &width, &height, &comp, STBI_rgb_alpha );
    }
    if (!data) return NULL;

    GLuint glTexId = 0;
    glGenTextures( 1, &glTexId );
    glBindTexture( GL_TEXTURE_2D, glTexId );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, srcFormat, format, data );

    stbi_image_free(data);

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
      data[i] = 0.0f;

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

  void SetShaderTexture( const char * szTextureName, Texture * tex )
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

  Texture * CreateA8TextureFromData( int w, int h, const unsigned char * data )
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

  void CopyBackbufferToTexture( Texture * tex )
  {
    glActiveTexture( GL_TEXTURE0 + ( (GLTexture *) tex )->unit );
    glBindTexture( GL_TEXTURE_2D, ( (GLTexture *) tex )->ID );
    glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 0, 0, nWidth, nHeight, 0 );
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

    glUseProgram(0);

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
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return true;
  }

}
