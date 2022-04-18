#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Scintilla.h"

#define DEVTYPE D3DDEVTYPE_HAL

#if defined(__MINGW32__) && !defined(D3D9b_SDK_VERSION)
#define D3D9b_SDK_VERSION 31
  // From https://docs.rs/winapi/0.2.8/i686-pc-windows-gnu/winapi/d3d9/constant.D3D9b_SDK_VERSION.html
#endif

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL (0x020E)
#endif

const char * shaderKeyword =
  " register packoffset static const"
  " break continue discard do for if else switch while case default return true false"
  " BINORMAL BINORMAL0 BINORMAL1 BINORMAL2 BINORMAL3 BINORMAL4 BINORMAL5 BINORMAL6 BINORMAL7"
  " BLENDINDICES BLENDINDICES0 BLENDINDICES1 BLENDINDICES2 BLENDINDICES3 BLENDINDICES4 BLENDINDICES5 BLENDINDICES6 BLENDINDICES7"
  " BLENDWEIGHT BLENDWEIGHT0 BLENDWEIGHT1 BLENDWEIGHT2 BLENDWEIGHT3 BLENDWEIGHT4 BLENDWEIGHT5 BLENDWEIGHT6 BLENDWEIGHT7"
  " COLOR COLOR0 COLOR1 COLOR2 COLOR3 COLOR4 COLOR5 COLOR6 COLOR7"
  " NORMAL NORMAL0 NORMAL1 NORMAL2 NORMAL3 NORMAL4 NORMAL5 NORMAL6 NORMAL7"
  " POSITION POSITION0 POSITION1 POSITION2 POSITION3 POSITION4 POSITION5 POSITION6 POSITION7"
  " POSITIONT"
  " PSIZE PSIZE0 PSIZE1 PSIZE2 PSIZE3 PSIZE4 PSIZE5 PSIZE6 PSIZE7"
  " TANGENT TANGENT0 TANGENT1 TANGENT2 TANGENT3 TANGENT4 TANGENT5 TANGENT6 TANGENT7"
  " TEXCOORD TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4 TEXCOORD5 TEXCOORD6 TEXCOORD7 TEXCOORD8 TEXCOORD9"
  " TEXCOORD0 TEXCOORD1 TEXCOORD2 TEXCOORD3 TEXCOORD4 TEXCOORD5 TEXCOORD6 TEXCOORD7 TEXCOORD8 TEXCOORD9"
  " SV_Coverage SV_Depth SV_DispatchThreadID SV_DomainLocation SV_GroupID SV_GroupIndex SV_GroupThreadID SV_GSInstanceID SV_InsideTessFactor SV_IsFrontFace SV_OutputControlPointID SV_POSITION SV_Position SV_RenderTargetArrayIndex SV_SampleIndex SV_TessFactor SV_ViewportArrayIndex SV_InstanceID SV_PrimitiveID SV_VertexID SV_TargetID"
  " SV_TARGET SV_Target SV_Target0 SV_Target1 SV_Target2 SV_Target3 SV_Target4 SV_Target5 SV_Target6 SV_Target7"
  " SV_ClipDistance0 SV_ClipDistance1 SV_ClipDistance2 SV_ClipDistance3 SV_ClipDistance4 SV_ClipDistance5 SV_ClipDistance6 SV_ClipDistance7"
  " SV_CullDistance0 SV_CullDistance1 SV_CullDistance2 SV_CullDistance3 SV_CullDistance4 SV_CullDistance5 SV_CullDistance6 SV_CullDistance7";

const char * shaderType = 
  "bool bool1 bool2 bool3 bool4"
  " bool1x1 bool1x2 bool1x3 bool1x4"
  " bool2x1 bool2x2 bool2x3 bool2x4"
  " bool3x1 bool3x2 bool3x3 bool3x4"
  " bool4x1 bool4x2 bool4x3 bool4x4"
  " int int1 int2 int3 int4"
  " int1x1 int1x2 int1x3 int1x4"
  " int2x1 int2x2 int2x3 int2x4"
  " int3x1 int3x2 int3x3 int3x4"
  " int4x1 int4x2 int4x3 int4x4"
  " uint uint1 uint2 uint3 uint4"
  " uint1x1 uint1x2 uint1x3 uint1x4"
  " uint2x1 uint2x2 uint2x3 uint2x4"
  " uint3x1 uint3x2 uint3x3 uint3x4"
  " uint4x1 uint4x2 uint4x3 uint4x4"
  " UINT UINT2 UINT3 UINT4"
  " dword dword1 dword2 dword3 dword4"
  " dword1x1 dword1x2 dword1x3 dword1x4"
  " dword2x1 dword2x2 dword2x3 dword2x4"
  " dword3x1 dword3x2 dword3x3 dword3x4"
  " dword4x1 dword4x2 dword4x3 dword4x4"
  " half half1 half2 half3 half4"
  " half1x1 half1x2 half1x3 half1x4"
  " half2x1 half2x2 half2x3 half2x4"
  " half3x1 half3x2 half3x3 half3x4"
  " half4x1 half4x2 half4x3 half4x4"
  " float float1 float2 float3 float4"
  " float1x1 float1x2 float1x3 float1x4"
  " float2x1 float2x2 float2x3 float2x4"
  " float3x1 float3x2 float3x3 float3x4"
  " float4x1 float4x2 float4x3 float4x4"
  " double double1 double2 double3 double4"
  " double1x1 double1x2 double1x3 double1x4"
  " double2x1 double2x2 double2x3 double2x4"
  " double3x1 double3x2 double3x3 double3x4"
  " double4x1 double4x2 double4x3 double4x4"
  " snorm unorm string void cbuffer struct"
  " Buffer AppendStructuredBfufer ByteAddressBuffer ConsumeStructuredBuffer StructuredBuffer"
  " RWBuffer RWByteAddressBuffer RWStructuredBuffer RWTexture1D RWTexture1DArray RWTexture2D RWTexture2DArray RWTexture3D"
  " InputPatch OutputPatch"
  " linear centroid nointerpolation noperspective sample"
  " sampler sampler1D sampler2D sampler3D samplerCUBE SamplerComparisonState SamplerState sampler_state"
  " AddressU AddressV AddressW BorderColor Filter MaxAnisotropy MaxLOD MinLOD MipLODBias ComparisonFunc ComparisonFilter"
  " texture Texture1D Texture1DArray Texture2D Texture2DArray Texture2DMS Texture2DMSArray Texture3D TextureCube";

const char * shaderBuiltin =
  "abs acos all AllMemoryBarrier AllMemoryBarrierWithGroupSync any asdouble"
  " asfloat asin asint asuint atan atan2 ceil clamp clip cos cosh countbits"
  " cross D3DCOLORtoUBYTE4 ddx ddx_coarse ddx_fine ddy ddy_coarse ddy_fine"
  " degrees determinant DeviceMemoryBarrier DeviceMemoryBarrierWithGroupSync"
  " distance dot dst EvaluateAttributeAtCentroid EvaluateAttributeAtSample"
  " EvaluateAttributeSnapped exp exp2 f16tof32 f32tof16 faceforward firstbithigh"
  " firstbitlow floor fmod frac frexp fwidth GetRenderTargetSampleCount"
  " GetRenderTargetSamplePosition GroupMemoryBarrier GroupMemoryBarrierWithGroupSync"
  " InterlockedAdd InterlockedAnd InterlockedCompareExchange InterlockedCompareStore"
  " InterlockedExchange InterlockedMax InterlockedMin InterlockedOr InterlockedXor"
  " isfinite isinf isnan ldexp length lerp lit log log10 log2 mad max min modf mul"
  " noise normalize pow Process2DQuadTessFactorsAvg Process2DQuadTessFactorsMax"
  " Process2DQuadTessFactorsMin ProcessIsolineTessFactors ProcessQuadTessFactorsAvg"
  " ProcessQuadTessFactorsMax ProcessQuadTessFactorsMin ProcessTriTessFactorsAvg"
  " ProcessTriTessFactorsMax ProcessTriTessFactorsMin radians rcp reflect refract"
  " reversebits round rsqrt saturate sign sin sincos sinh smoothstep sqrt step"
  " tan tanh tex1D tex1Dbias tex1Dgrad tex1Dlod tex1Dproj tex2D tex2Dbias"
  " tex2Dgrad tex2Dlod tex2Dproj tex3D tex3Dbias tex3Dgrad tex3Dlod tex3Dproj"
  " texCUBE texCUBEbias texCUBEgrad texCUBElod texCUBEproj transpose trunc";

namespace Renderer
{
  const char * defaultShaderFilename = "shader.dx9.hlsl";
  const char defaultShader[65536] = 
    "texture texTFFT; sampler1D texFFT = sampler_state { Texture = <texTFFT>; }; \n"
    "// towards 0.0 is bass / lower freq, towards 1.0 is higher / treble freq\n"
    "texture texFFTSmoothedT; sampler1D texFFTSmoothed = sampler_state { Texture = <texFFTSmoothedT>; }; \n"
    "// this one has longer falloff and less harsh transients\n"
    "texture texFFTIntegratedT; sampler1D texFFTIntegrated = sampler_state { Texture = <texFFTIntegratedT>; }; \n"
    "// this is continually increasing\n"
    "texture texPreviousFrameT; sampler2D texPreviousFrame = sampler_state { Texture = <texPreviousFrameT>; }; \n"
    "// screenshot of the previous frame\n"
    "\n"
    "{%textures:begin%}" // leave off \n here
    "texture raw{%textures:name%}; sampler2D {%textures:name%} = sampler_state { Texture = <raw{%textures:name%}>; };\n"
    "{%textures:end%}"
    "\n"
    "{%midi:begin%}" // leave off \n here
    "float {%midi:name%};\n"
    "{%midi:end%}"
    "float fGlobalTime; // in seconds\n"
    "float fFrameTime; // duration of the last frame, in seconds\n"
    "float2 v2Resolution; // viewport resolution (in pixels)\n"
    "\n"
    "float4 plas( float2 v, float time )\n"
    "{\n"
    "\tfloat c = 0.5 + sin( v.x * 10.0 ) + cos( sin( time + v.y ) * 20.0 );\n"
    "\treturn float4( sin(c * 0.2 + cos(time)), c * 0.15, cos( c * 0.1 + time / .4 ) * .25, 1.0 );\n"
    "}\n"
    "\n"
    "float4 main( float2 TexCoord : TEXCOORD0 ) : COLOR0\n"
    "{\n"
    "\tfloat2 uv = TexCoord;\n"
    "\tuv -= 0.5;\n"
    "\tuv /= float2(v2Resolution.y / v2Resolution.x, 1);"
    "\n"
    "\tfloat2 m;\n"
    "\tm.x = atan(uv.x / uv.y) / 3.14;\n"
    "\tm.y = 1 / length(uv) * .2;\n"
    "\tfloat d = m.y;\n"
    "\n"
    "\tfloat f = tex1D( texFFT, d ).r * 100;\n"
    "\tm.x += sin( fGlobalTime ) * 0.1;\n"
    "\tm.y += fGlobalTime * 0.25;\n"
    "\n"
    "\tfloat4 t = plas( m * 3.14, fGlobalTime ) / d;\n"
    "\tt = saturate( t );\n"
    "\treturn f + t;\n"
    "}";
  char defaultVertexShader[65536] = 
    "struct VS_INPUT_PP { float3 Pos : POSITION0; float2 TexCoord : TEXCOORD0; };\n"
    "struct VS_OUTPUT_PP { float4 Pos : POSITION0; float2 TexCoord : TEXCOORD0; };\n"
    "\n"
    "VS_OUTPUT_PP main( VS_INPUT_PP In )\n"
    "{\n"
    "\tVS_OUTPUT_PP Out;\n"
    "\tOut.Pos = float4( In.Pos, 1.0 );\n"
    "\tOut.TexCoord = In.TexCoord;\n"
    "\treturn Out;\n"
    "}\n";

  bool run = true;

  LPDIRECT3D9 pD3D = NULL;
  LPDIRECT3DDEVICE9 pDevice = NULL;
  LPD3DXCONSTANTTABLE pConstantTable = NULL;
  LPDIRECT3DVERTEXSHADER9 pVertexShader = NULL;
  LPDIRECT3DPIXELSHADER9 theShader = NULL;
  LPDIRECT3DSURFACE9 pBackBuffer = NULL;
  LPDIRECT3DSURFACE9 pFrameGrabTexture = NULL;

  int nWidth = 0;
  int nHeight = 0;
  HWND hWnd = NULL;

  KeyEvent keyEventBuffer[512];
  int keyEventBufferCount = 0;
  MouseEvent mouseEventBuffer[512];
  int mouseEventBufferCount = 0;

  LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
  {
    switch (uMsg) 
    {
    case WM_KEYDOWN: 
      {
        int sciKey = 0;
        switch(wParam)
        {
        case VK_DOWN:         sciKey = SCK_DOWN;      break;
        case VK_UP:           sciKey = SCK_UP;        break;
        case VK_LEFT:         sciKey = SCK_LEFT;      break;
        case VK_RIGHT:        sciKey = SCK_RIGHT;     break;
        case VK_HOME:         sciKey = SCK_HOME;      break;
        case VK_END:          sciKey = SCK_END;       break;
        case VK_PRIOR:        sciKey = SCK_PRIOR;     break;
        case VK_NEXT:         sciKey = SCK_NEXT;      break;
        case VK_DELETE:       sciKey = SCK_DELETE;    break;
        case VK_INSERT:       sciKey = SCK_INSERT;    break;
        case VK_ESCAPE:       sciKey = SCK_ESCAPE;    break;
        case VK_BACK:         sciKey = SCK_BACK;      break;
        case VK_TAB:          sciKey = SCK_TAB;       break;
        case VK_RETURN:       sciKey = SCK_RETURN;    break;
//         case VK_KP_PLUS:      sciKey = SCK_ADD;       break;
//         case VK_KP_MINUS:     sciKey = SCK_SUBTRACT;  break;
//         case VK_KP_DIVIDE:    sciKey = SCK_DIVIDE;    break;
//         case VK_LSUPER:       sciKey = SCK_WIN;       break;
//         case VK_RSUPER:       sciKey = SCK_RWIN;      break;
        case VK_MENU:         sciKey = SCK_MENU;      break;
//         case VK_SLASH:        sciKey = '/';           break;
//         case VK_ASTERISK:     sciKey = '`';           break;
//         case VK_LEFTBRACKET:  sciKey = '[';           break;
//         case VK_BACKSLASH:    sciKey = '\\';          break;
//         case VK_RIGHTBRACKET: sciKey = ']';           break;
        case VK_F2:         sciKey = 283;      break;
        case VK_F5:         sciKey = 286;      break;
        case VK_F11:        sciKey = 292;      break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LMENU:
        case VK_RMENU:
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
          sciKey = 0;
          break;
        default:
          sciKey = wParam;
        }
        if (sciKey)
        {
          keyEventBuffer[keyEventBufferCount].ctrl  = GetAsyncKeyState( VK_LCONTROL ) || GetAsyncKeyState( VK_RCONTROL );
          keyEventBuffer[keyEventBufferCount].alt   = GetAsyncKeyState( VK_LMENU ) || GetAsyncKeyState( VK_RMENU );
          keyEventBuffer[keyEventBufferCount].shift = GetAsyncKeyState( VK_LSHIFT ) || GetAsyncKeyState( VK_RSHIFT );
          keyEventBuffer[keyEventBufferCount].scanCode = sciKey;
          keyEventBuffer[keyEventBufferCount].character = 0;
          keyEventBufferCount++;
        }
        //pKeys[wParam] = 1;
      } break;
    case WM_CHAR: 
      {
        if (wParam >= 32)
        {
          keyEventBuffer[keyEventBufferCount].ctrl  = GetAsyncKeyState( VK_LCONTROL ) || GetAsyncKeyState( VK_RCONTROL );
          keyEventBuffer[keyEventBufferCount].alt   = GetAsyncKeyState( VK_LMENU ) || GetAsyncKeyState( VK_RMENU );
          keyEventBuffer[keyEventBufferCount].shift = GetAsyncKeyState( VK_LSHIFT ) || GetAsyncKeyState( VK_RSHIFT );
          keyEventBuffer[keyEventBufferCount].scanCode = 0;
          keyEventBuffer[keyEventBufferCount].character = wParam;
          keyEventBufferCount++;
        }
        //pKeys[wParam] = 1;
      } break;

    case WM_KEYUP: 
      {
        //pKeys[wParam] = 0;
      } break;

    case WM_LBUTTONDOWN: 
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_DOWN;
        mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT;
        mouseEventBuffer[mouseEventBufferCount].x = GET_X_LPARAM(lParam);
        mouseEventBuffer[mouseEventBufferCount].y = GET_Y_LPARAM(lParam);
        mouseEventBufferCount++;
      } break;

    case WM_MOUSEMOVE: 
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_MOVE;
        mouseEventBuffer[mouseEventBufferCount].x = GET_X_LPARAM(lParam);
        mouseEventBuffer[mouseEventBufferCount].y = GET_Y_LPARAM(lParam);
        mouseEventBufferCount++;
      } break;

    case WM_LBUTTONUP: 
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_UP;
        mouseEventBuffer[mouseEventBufferCount].button = MOUSEBUTTON_LEFT;
        mouseEventBuffer[mouseEventBufferCount].x = GET_X_LPARAM(lParam);
        mouseEventBuffer[mouseEventBufferCount].y = GET_Y_LPARAM(lParam);
        mouseEventBufferCount++;
      } break;

    case WM_MOUSEWHEEL:
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_SCROLL;
        mouseEventBuffer[mouseEventBufferCount].x = 0;
        mouseEventBuffer[mouseEventBufferCount].y = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        mouseEventBufferCount++;
      } break;

    case WM_MOUSEHWHEEL:
      {
        mouseEventBuffer[mouseEventBufferCount].eventType = MOUSEEVENTTYPE_SCROLL;
        mouseEventBuffer[mouseEventBufferCount].x = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        mouseEventBuffer[mouseEventBufferCount].y = 0;
        mouseEventBufferCount++;
      } break;

    case WM_SYSCOMMAND: 
      {
        switch (wParam) 
        {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER: 
          {
            return 0;
          }
        }
      } break;

    case WM_CLOSE: 
      {
        run = false;
      } break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }

  bool InitWindow(RENDERER_SETTINGS * pSetup) 
  {
    WNDCLASS WC;

    WC.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WC.lpfnWndProc = &WndProc;
    WC.cbClsExtra = 0;
    WC.cbWndExtra = 0;
    WC.hInstance = GetModuleHandle(NULL);
    //WC.hIcon = LoadIcon(setup->hInstance,MAKEINTRESOURCE(IDI_ICON1));
    WC.hIcon = NULL;
    WC.hCursor = LoadCursor(NULL, IDC_ARROW);
    WC.hbrBackground = NULL;
    WC.lpszMenuName = NULL;
    WC.lpszClassName = _T("fwzwnd");
    if(!RegisterClass(&WC)) return 0;

    DWORD wExStyle = WS_EX_APPWINDOW;
    DWORD wStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if (pSetup->windowMode == RENDERER_WINDOWMODE_WINDOWED) wStyle |= WS_OVERLAPPED | WS_CAPTION;

    RECT wr={0,0,pSetup->nWidth,pSetup->nHeight};
    AdjustWindowRectEx(&wr, wStyle, FALSE, wExStyle);

    hWnd = CreateWindowEx(wExStyle,_T("fwzwnd"),_T("BONZOMATIC - Direct3D 9.0c edition"),wStyle,
      (GetSystemMetrics(SM_CXSCREEN) - pSetup->nWidth )/2,
      (GetSystemMetrics(SM_CYSCREEN) - pSetup->nHeight)/2,
      wr.right-wr.left, wr.bottom-wr.top,
      NULL, NULL, WC.hInstance, NULL);

    if (!hWnd) 
      return false;

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    return true;
  }

  D3DPRESENT_PARAMETERS d3dpp;
  bool InitDirect3D(RENDERER_SETTINGS * pSetup) 
  {
    pD3D = Direct3DCreate9(D3D9b_SDK_VERSION);
    if (!pD3D) 
    {
      printf("[Renderer] Direct3DCreate9 failed\n");
      return false;
    }

    nWidth  = pSetup->nWidth;
    nHeight = pSetup->nHeight;

    ZeroMemory(&d3dpp,sizeof(d3dpp));

    d3dpp.SwapEffect     = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow  = hWnd;
    d3dpp.Windowed       = pSetup->windowMode != RENDERER_WINDOWMODE_FULLSCREEN;
    d3dpp.PresentationInterval = pSetup->bVsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

    d3dpp.BackBufferCount  = 1;

    d3dpp.BackBufferWidth  = pSetup->nWidth;
    d3dpp.BackBufferHeight = pSetup->nHeight;

    D3DDISPLAYMODE d3ddm;
    if( FAILED( pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
    {
      printf("[Renderer] GetAdapterDisplayMode failed\n");
      return false;
    }

    static D3DFORMAT pBackbufferFormats32[] = {
      D3DFMT_X8R8G8B8,
      D3DFMT_A8R8G8B8,
      D3DFMT_UNKNOWN
    };
    D3DFORMAT * pFormats = pBackbufferFormats32;
    for (int i=0; pFormats[i] != D3DFMT_UNKNOWN; i++)
      if ( SUCCEEDED(pD3D->CheckDeviceType( D3DADAPTER_DEFAULT, DEVTYPE, d3ddm.Format, pFormats[i], d3dpp.Windowed )) )
        d3dpp.BackBufferFormat = pFormats[i];

    if (d3dpp.BackBufferFormat == D3DFMT_UNKNOWN) 
    {
      printf("[Renderer] No suitable backbuffer format found\n");
      return false;
    }

    d3dpp.EnableAutoDepthStencil = FALSE;

    static D3DMULTISAMPLE_TYPE pMultisampleTypes[]=
    {
      D3DMULTISAMPLE_2_SAMPLES,
      D3DMULTISAMPLE_4_SAMPLES,
      D3DMULTISAMPLE_6_SAMPLES,
      D3DMULTISAMPLE_8_SAMPLES,
      D3DMULTISAMPLE_NONE
    };

    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

    HRESULT h;
    h = pD3D->CreateDevice( D3DADAPTER_DEFAULT, DEVTYPE, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
    if (h != D3D_OK) 
    {
      h = pD3D->CreateDevice( D3DADAPTER_DEFAULT, DEVTYPE, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
      if(h != D3D_OK) 
      {
        printf("[Renderer] CreateDevice failed: %08X\n",h);
        return false;
      }
    }

    pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    for (int x=0; x<4; x++) 
    {
      pDevice->SetSamplerState( x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    }

    pDevice->GetRenderTarget( 0, &pBackBuffer );

    return 1;
  }

  LPDIRECT3DVERTEXBUFFER9 pFullscreenQuadVB = NULL;
  LPDIRECT3DVERTEXBUFFER9 pGUIQuadVB = NULL;
  LPDIRECT3DVERTEXDECLARATION9 pFullscreenQuadVertexDecl = NULL;

#define GUIQUADVB_SIZE (128*6)

  bool Open( RENDERER_SETTINGS * settings )
  {
    if (!InitWindow(settings))
    {
      printf("[Renderer] InitWindow failed\n");
      return false;
    }

    if (!InitDirect3D(settings))
    {
      printf("[Renderer] InitDirect3D failed\n");
      return false;
    }

    static D3DVERTEXELEMENT9 pFullscreenQuadElements[] = 
    {
      { 0, 0*sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 3*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      D3DDECL_END()
    };

    static float pQuad[] = 
    {
      -1.0, -1.0,  0.0, 0.0 + 0.5 / (float)nWidth, 0.0 + 0.5 / (float)nHeight,
      -1.0,  1.0,  0.0, 0.0 + 0.5 / (float)nWidth, 1.0 + 0.5 / (float)nHeight,
       1.0, -1.0,  0.0, 1.0 + 0.5 / (float)nWidth, 0.0 + 0.5 / (float)nHeight,
       1.0,  1.0,  0.0, 1.0 + 0.5 / (float)nWidth, 1.0 + 0.5 / (float)nHeight,
    };

    pDevice->CreateVertexBuffer( 4 * 5 * sizeof(float), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pFullscreenQuadVB, NULL);
    void * v;
    pFullscreenQuadVB->Lock( 0, 4 * 5 * sizeof(float), &v, NULL );
    CopyMemory( v, pQuad, 4 * 5 * sizeof(float) );
    pFullscreenQuadVB->Unlock();

    pDevice->CreateVertexDeclaration( pFullscreenQuadElements, &pFullscreenQuadVertexDecl );

    //////////////////////////////////////////////////////////////////////////

    pDevice->CreateVertexBuffer( GUIQUADVB_SIZE * 6 * sizeof(float), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pGUIQuadVB, NULL);

    //////////////////////////////////////////////////////////////////////////

    LPD3DXBUFFER pShader = NULL;
    LPD3DXBUFFER pErrors = NULL;

    if (D3DXCompileShader( defaultVertexShader, strlen(defaultVertexShader), NULL, NULL, "main", "vs_3_0", NULL, &pShader, &pErrors, NULL ) != D3D_OK)
    {
      printf("[Renderer] D3DXCompileShader failed\n");
      return false;
    }

    if (pDevice->CreateVertexShader( (DWORD*)pShader->GetBufferPointer(), &pVertexShader ) != D3D_OK)
    {
      printf("[Renderer] CreateVertexShader failed\n");
      return false;
    }

    if (pDevice->CreateOffscreenPlainSurface( settings->nWidth, settings->nHeight, d3dpp.BackBufferFormat, D3DPOOL_SYSTEMMEM, &pFrameGrabTexture, NULL) != D3D_OK)
    {
      printf("[Renderer] CreateOffscreenPlainSurface failed\n");
      return false;
    }

    return true;
  }

  unsigned int nCacheFlushCount = 0;
  void StartFrame()
  {
    nCacheFlushCount = 0;
    MSG msg;
    if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) 
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }

    pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0xFF808080, 1.0f, 0 );
    pDevice->BeginScene();
  }
  void EndFrame()
  {
    pDevice->EndScene();
    pDevice->Present( NULL, NULL, NULL, NULL );
  }
  bool WantsToQuit()
  {
    return !run;
  }
  void Close()
  {
    if (pFullscreenQuadVB) pFullscreenQuadVB->Release();
    if (pFullscreenQuadVertexDecl) pFullscreenQuadVertexDecl->Release();
    if (pGUIQuadVB) pGUIQuadVB->Release();
    if (pVertexShader) pVertexShader->Release();
    if (theShader) theShader->Release();
    if (pDevice) pDevice->Release();
    if (pD3D) pD3D->Release();
    if (hWnd) 
    {
      DestroyWindow(hWnd);
      UnregisterClass(_T("fwzwnd"),GetModuleHandle(NULL));
    }
  }

  void RenderFullscreenQuad()
  {
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

    pDevice->SetVertexShader( pVertexShader );
    pDevice->SetPixelShader( theShader );
    
    pDevice->SetVertexDeclaration( pFullscreenQuadVertexDecl );
    pDevice->SetStreamSource( 0, pFullscreenQuadVB, 0, sizeof(float) * 5 ); 
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  }

  bool ReloadShader( const char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    LPD3DXBUFFER pShader = NULL;
    LPD3DXBUFFER pErrors = NULL;

    if (D3DXCompileShader( szShaderCode, nShaderCodeSize, NULL, NULL, "main", "ps_3_0", NULL, &pShader, &pErrors, &pConstantTable ) != D3D_OK)
    {
      memset( szErrorBuffer, 0, nErrorBufferSize );
      strncpy( szErrorBuffer, (const char*)pErrors->GetBufferPointer(), nErrorBufferSize - 1 );
      return false;
    }

    if (theShader) 
    {
      theShader->Release();
      theShader = NULL;
    }

    if (pDevice->CreatePixelShader( (DWORD*)pShader->GetBufferPointer(), &theShader ) != D3D_OK)
    {
      return false;
    }

    return true;
  }

  void SetShaderConstant( const char * szConstName, float x )
  {
    pConstantTable->SetFloat( pDevice, szConstName, x );
  }

  static D3DXVECTOR4 SetShaderConstant_VEC4;
  void SetShaderConstant( const char * szConstName, float x, float y )
  {
    SetShaderConstant_VEC4.x = x;
    SetShaderConstant_VEC4.y = y;
    SetShaderConstant_VEC4.z = 0;
    SetShaderConstant_VEC4.w = 0;
    pConstantTable->SetVector( pDevice, szConstName, &SetShaderConstant_VEC4 );
  }

  struct DX9Texture : public Texture
  {
    LPDIRECT3DTEXTURE9 pTexture;
  };

  int textureUnit = 0;

  Renderer::Texture * CreateRGBA8Texture()
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    pDevice->CreateTexture( nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pTex, NULL );

    if ( !pTex )
      return NULL;

    DX9Texture * tex = new DX9Texture();
    tex->pTexture = pTex;
    tex->width = nWidth;
    tex->height = nHeight;
    tex->type = TEXTURETYPE_2D;
    return tex;

  }

  Texture * CreateRGBA8TextureFromFile( const char * szFilename )
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    D3DXIMAGE_INFO info;
    HRESULT h = D3DXCreateTextureFromFileExA(
      pDevice,
      szFilename,
      D3DX_DEFAULT_NONPOW2,
      D3DX_DEFAULT_NONPOW2,
      0,
      NULL,
      D3DFMT_FROM_FILE,
      D3DPOOL_DEFAULT,
      D3DX_DEFAULT,
      D3DX_DEFAULT,
      NULL,
      &info,
      NULL,
      &pTex);

    if (!pTex)
      return NULL;

    DX9Texture * tex = new DX9Texture();
    tex->pTexture = pTex;
    tex->width = info.Width;
    tex->height = info.Height;
    tex->type = TEXTURETYPE_2D;
    return tex;
  }

  Texture * Create1DR32Texture( int w )
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    pDevice->CreateTexture( w, 1, 0, D3DUSAGE_DYNAMIC, D3DFMT_R32F, D3DPOOL_DEFAULT, &pTex, NULL );

    if (!pTex)
      return NULL;

    D3DLOCKED_RECT rect;
    pTex->LockRect( 0, &rect, NULL, NULL );
    memset( rect.pBits, 0, w * sizeof(float) );
    pTex->UnlockRect(0);

    DX9Texture * tex = new DX9Texture();
    tex->pTexture = pTex;
    tex->width = w;
    tex->height = 1;
    tex->type = TEXTURETYPE_1D;
    return tex;
  }

  void SetShaderTexture( const char * szTextureName, Texture * tex )
  {
    int idx = pConstantTable->GetSamplerIndex( szTextureName );
    if (idx >= 0)
    {
      pDevice->SetSamplerState( idx, D3DSAMP_SRGBTEXTURE, TRUE );
      pDevice->SetSamplerState( idx, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
      pDevice->SetSamplerState( idx, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
      pDevice->SetTexture( idx, ((DX9Texture *)tex)->pTexture );
    }
  }

  bool UpdateR32Texture( Texture * tex, float * data )
  {
    LPDIRECT3DTEXTURE9 pTex = ((DX9Texture *)tex)->pTexture;
    
    D3DLOCKED_RECT rect;
    pTex->LockRect( 0, &rect, NULL, D3DLOCK_DISCARD );
    memcpy( rect.pBits, data, tex->width * sizeof(float) );
    pTex->UnlockRect(0);

    return true;
  }

  Texture * CreateA8TextureFromData( int w, int h, const unsigned char * data )
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    pDevice->CreateTexture( w, h, 0, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTex, NULL );

    if (!pTex)
      return NULL;

    D3DLOCKED_RECT rect;
    pTex->LockRect( 0, &rect, NULL, NULL );
    const unsigned char * src = data;
    unsigned char * dst = (unsigned char *)rect.pBits;
    for (int i=0; i<h; i++)
    {
      const unsigned char * srcLine = src;
      unsigned int * dstLine = (unsigned int *)dst;
      for (int j=0; j<w; j++)
      {
        *dstLine = (*srcLine << 24) | 0xFFFFFF;
        srcLine++;
        dstLine++;
      }
      src += w * sizeof(unsigned char);
      dst += rect.Pitch;
    }
    pTex->UnlockRect(0);

    DX9Texture * tex = new DX9Texture();
    tex->pTexture = pTex;
    tex->width = w;
    tex->height = h;
    tex->type = TEXTURETYPE_1D;
    return tex;
  }

  void ReleaseTexture( Texture * tex )
  {
    ( (DX9Texture *) tex )->pTexture->Release();
    delete tex;
  }

  void CopyBackbufferToTexture( Texture * tex )
  {
    LPDIRECT3DTEXTURE9 pTex = ( (DX9Texture *) tex )->pTexture;
    LPDIRECT3DSURFACE9 pSurf = NULL;
    pTex->GetSurfaceLevel( 0, &pSurf );
    if ( pSurf )
    {
      HRESULT res = pDevice->StretchRect( pBackBuffer, NULL, pSurf, NULL, D3DTEXF_LINEAR );
      pSurf->Release();
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // text rendering

  void StartTextRendering()
  {
    pDevice->SetVertexShader( NULL );
    pDevice->SetPixelShader( NULL );
    pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
    //pDevice->SetVertexDeclaration( pGUIVertexDecl );
    pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );
    pDevice->SetTransform( D3DTS_VIEW, &mat );
    pDevice->SetTransform( D3DTS_WORLD, &mat );
    D3DXMatrixOrthoOffCenterLH( (D3DXMATRIX*)&mat, 0, nWidth, nHeight, 0, -1.0f, 1.0f );
    pDevice->SetTransform( D3DTS_PROJECTION, &mat );

    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
  }

  int bufferPointer = 0;
  unsigned char buffer[GUIQUADVB_SIZE * sizeof(float) * 6];
  bool lastModeIsQuad = true;
  void __FlushRenderCache()
  {
    if (!bufferPointer) return;

    nCacheFlushCount++;
    void * v = NULL;
    pGUIQuadVB->Lock( 0, bufferPointer * sizeof(float) * 6, &v, NULL );
    CopyMemory( v, buffer, bufferPointer * sizeof(float) * 6 );
    pGUIQuadVB->Unlock();

    pDevice->SetStreamSource( 0, pGUIQuadVB, 0, sizeof(float) * 6 ); 
    if (lastModeIsQuad)
      pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, bufferPointer / 3 );
    else
      pDevice->DrawPrimitive( D3DPT_LINELIST, 0, bufferPointer / 2 );

    bufferPointer = 0;
  }
  inline unsigned int _dxARGBtoABGR(unsigned int abgr)
  {
    return (abgr&0xff00ff00)+((abgr<<16)&0x00ff0000)+((abgr>>16)&0x000000ff);
  }
  Texture * lastTexture = NULL;
  void __WriteVertexToBuffer( const Vertex & v )
  {
    if (bufferPointer >= GUIQUADVB_SIZE)
    {
      __FlushRenderCache();
    }

    float * f = (float*)(buffer + bufferPointer * sizeof(float) * 6);
    *(f++) = v.x;
    *(f++) = v.y;
    *(f++) = 0.0;
    *(unsigned int *)(f++) = _dxARGBtoABGR( v.c );
    *(f++) = v.u + (lastTexture ? (0.5 / (float)lastTexture->width ) : 0.0);
    *(f++) = v.v + (lastTexture ? (0.5 / (float)lastTexture->height) : 0.0);
    bufferPointer++;
  }
  void BindTexture( Texture * tex )
  {
    if (lastTexture != tex)
    {
      __FlushRenderCache();
      lastTexture = tex;
      pDevice->SetTexture( 0, tex ? ((DX9Texture *)tex)->pTexture : NULL );
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
    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );
    mat._41 = rect.left;
    mat._42 = rect.top;
    pDevice->SetTransform( D3DTS_WORLD, &mat );

    RECT rc = { rect.left, rect.top, rect.right, rect.bottom };
    pDevice->SetScissorRect( &rc );
  }
  void EndTextRendering()
  {
    __FlushRenderCache();
    pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, false );
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );
  }

  //////////////////////////////////////////////////////////////////////////

  bool GrabFrame( void * pPixelBuffer )
  {
    if (!pFrameGrabTexture)
      return false;

    if (pDevice->GetRenderTargetData( pBackBuffer, pFrameGrabTexture ) != D3D_OK)
      return false;

    D3DLOCKED_RECT rect;
    if (pFrameGrabTexture->LockRect( &rect, NULL, NULL ) != D3D_OK)
      return false;
    
    unsigned char* pSrc = (unsigned char*)rect.pBits;
    unsigned char* pDst = (unsigned char*)pPixelBuffer;
    for( int i = 0; i < nHeight; i++ )
    {
      unsigned int* pSrc32 = (unsigned int*)pSrc;
      unsigned int* pDst32 = (unsigned int*)pDst;
      for(int j=0; j < nWidth; j++)
        pDst32[j] = (pSrc32[j] & 0x00FF00) | ((pSrc32[j] >> 16) & 0xFF) | ((pSrc32[j] & 0xFF) << 16) | 0xFF000000;

      pSrc += rect.Pitch;
      pDst += nWidth * 4;
    }

    pFrameGrabTexture->UnlockRect();

    return true;
  }
}
