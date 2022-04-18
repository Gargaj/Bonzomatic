#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <float.h>

#include <d3d11.h>
#include <D3Dcompiler.h>

#include "../Renderer.h"

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL (0x020E)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Scintilla.h"

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
  const char * defaultShaderFilename = "shader.dx11.hlsl";
  const char defaultShader[65536] =
    "{%textures:begin%}" // leave off \n here
    "Texture2D {%textures:name%};\n"
    "{%textures:end%}" // leave off \n here
    "Texture1D texFFT; // towards 0.0 is bass / lower freq, towards 1.0 is higher / treble freq\n"
    "Texture1D texFFTSmoothed; // this one has longer falloff and less harsh transients\n"
    "Texture1D texFFTIntegrated; // this is continually increasing\n"
    "Texture2D texPreviousFrame; // screenshot of the previous frame\n"
    "SamplerState smp;\n"
    "\n"
    "cbuffer constants\n"
    "{\n"
    "\tfloat fGlobalTime; // in seconds\n"
    "\tfloat2 v2Resolution; // viewport resolution (in pixels)\n"
    "\tfloat fFrameTime; // duration of the last frame, in seconds\n"
    "{%midi:begin%}"
    "\tfloat {%midi:name%};\n"
    "{%midi:end%}"
    "}\n"
    "\n"
    "float4 plas( float2 v, float time )\n"
    "{\n"
    "\tfloat c = 0.5 + sin( v.x * 10.0 ) + cos( sin( time + v.y ) * 20.0 );\n"
    "\treturn float4( sin(c * 0.2 + cos(time)), c * 0.15, cos( c * 0.1 + time / .4 ) * .25, 1.0 );\n"
    "}\n"
    "\n"
    "float4 main( float4 position : SV_POSITION, float2 TexCoord : TEXCOORD ) : SV_TARGET\n"
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
    "\tfloat f = texFFT.Sample( smp, d ).r * 100;\n"
    "\tm.x += sin( fGlobalTime ) * 0.1;\n"
    "\tm.y += fGlobalTime * 0.25;\n"
    "\n"
    "\tfloat4 t = plas( m * 3.14, fGlobalTime ) / d;\n"
    "\tt = saturate( t );\n"
    "\treturn f + t;\n"
    "}";

  char defaultVertexShader[65536] = 
    "struct VS_INPUT_PP { float3 Pos : POSITION; float2 TexCoord : TEXCOORD; };\n"
    "struct VS_OUTPUT_PP { float4 Pos : SV_POSITION; float2 TexCoord : TEXCOORD; };\n"
    "\n"
    "VS_OUTPUT_PP main( VS_INPUT_PP In )\n"
    "{\n"
    "\tVS_OUTPUT_PP Out;\n"
    "\tOut.Pos = float4( In.Pos, 1.0 );\n"
    "\tOut.TexCoord = In.TexCoord;\n"
    "\treturn Out;\n"
    "}\n";

  bool run = true;

  IDXGISwapChain * pSwapChain = NULL;
  ID3D11Device * pDevice = NULL;
  ID3D11DeviceContext * pContext = NULL;
  ID3D11RenderTargetView * pRenderTarget = NULL;
  ID3D11VertexShader * pVertexShader = NULL;
  ID3D11PixelShader * theShader = NULL;
  ID3D11ShaderReflection * pShaderReflection = NULL;
  ID3D11Texture2D * pBackBuffer = NULL;
  ID3D11Texture2D * pFrameGrabTexture = NULL;

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

    hWnd = CreateWindowEx(wExStyle,_T("fwzwnd"),_T("BONZOMATIC - Direct3D 11.0 edition"),wStyle,
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

  bool bVsync = false;
  bool InitDirect3D(RENDERER_SETTINGS * pSetup) 
  {
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.BufferCount = 1;
    desc.BufferDesc.Width = pSetup->nWidth;
    desc.BufferDesc.Height = pSetup->nHeight;
    desc.BufferDesc.Format = format;
    if (pSetup->bVsync)
    {
      bVsync = true;
      IDXGIFactory1 * pFactory = NULL;
      HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);
      if (pFactory)
      {
        IDXGIAdapter1 * pAdapter = NULL;
        pFactory->EnumAdapters1( 0, &pAdapter );
        if (pAdapter)
        {
          IDXGIOutput * pOutput = NULL;
          pAdapter->EnumOutputs( 0, &pOutput );
          if (pOutput)
          {
            unsigned int nModeCount = 0;
            pOutput->GetDisplayModeList( format, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &nModeCount, NULL);

            DXGI_MODE_DESC * pModes = new DXGI_MODE_DESC[ nModeCount ];
            pOutput->GetDisplayModeList( format, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &nModeCount, pModes);

            for (unsigned int i=0; i<nModeCount; i++)
            {
              if (pModes[i].Width == pSetup->nWidth && pModes[i].Height == pSetup->nHeight)
              {
                desc.BufferDesc = pModes[i];
                break;
              }
            }
            delete[] pModes;

            pOutput->Release();
          }

          pAdapter->Release();
        }

        pFactory->Release();
      }
    }
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hWnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = pSetup->windowMode != RENDERER_WINDOWMODE_FULLSCREEN;

    DWORD deviceCreationFlags = 0;
#ifdef _DEBUG
    deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    if (D3D11CreateDeviceAndSwapChain(
      NULL,
      D3D_DRIVER_TYPE_HARDWARE,
      NULL,
      deviceCreationFlags,
      NULL,
      NULL,
      D3D11_SDK_VERSION,
      &desc,
      &pSwapChain,
      &pDevice,
      NULL,
      &pContext) != S_OK)
    {
      printf("[Renderer] D3D11CreateDeviceAndSwapChain failed\n");
      return false;
    }

    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

    pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTarget);
    pBackBuffer->Release();

    pContext->OMSetRenderTargets(1, &pRenderTarget, NULL);

    // create staging texture for frame grabbing

    D3D11_TEXTURE2D_DESC description;
    pBackBuffer->GetDesc( &description );
    description.BindFlags = 0;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    description.Usage = D3D11_USAGE_STAGING;

    HRESULT hr = pDevice->CreateTexture2D( &description, NULL, &pFrameGrabTexture );

    return true;
  }

  ID3D11Buffer * pFullscreenQuadVB = NULL;
  ID3D11InputLayout * pFullscreenQuadLayout = NULL;
  ID3D11SamplerState * pFullscreenQuadSamplerState = NULL;
  ID3D11Buffer * pFullscreenQuadConstantBuffer = NULL;
#define FULLSCREENQUADCONSTANTS_SIZE 512
  unsigned char pFullscreenQuadConstants[FULLSCREENQUADCONSTANTS_SIZE];
  ID3D11BlendState* pFullscreenQuadBlendState = NULL;
  ID3D11RasterizerState * pFullscreenQuadRasterizerState = NULL;

  bool InitD3D11QuadRendering( RENDERER_SETTINGS * settings )
  {
    ID3DBlob * pCode = NULL;
    ID3DBlob * pErrors = NULL;
    if (D3DCompile( defaultVertexShader, strlen(defaultVertexShader), NULL, NULL, NULL, "main", "vs_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
    {
      printf("[Renderer] D3DCompile failed\n");
      return false;
    }

    if (pDevice->CreateVertexShader( pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pVertexShader ) != S_OK)
    {
      printf("[Renderer] CreateVertexShader failed\n");
      return false;
    }

    //////////////////////////////////////////////////////////////////////////

    static float pQuad[] = {
      -1.0, -1.0,  0.5, 0.0, 0.0,
      -1.0,  1.0,  0.5, 0.0, 1.0,
       1.0, -1.0,  0.5, 1.0, 0.0,
       1.0,  1.0,  0.5, 1.0, 1.0,
    };

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

    desc.ByteWidth = sizeof(float) * 5 * 4;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData, sizeof(D3D11_SUBRESOURCE_DATA));
    subData.pSysMem = pQuad;

    if (pDevice->CreateBuffer(&desc, &subData, &pFullscreenQuadVB) != S_OK)
    {
      printf("[Renderer] CreateBuffer failed\n");
      return false;
    }

    static D3D11_INPUT_ELEMENT_DESC pQuadDesc[] =
    {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (pDevice->CreateInputLayout( pQuadDesc, 2, pCode->GetBufferPointer(), pCode->GetBufferSize(), &pFullscreenQuadLayout) != S_OK)
    {
      printf("[Renderer] CreateInputLayout failed\n");
      return false;
    }

    //////////////////////////////////////////////////////////////////////////

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width  = settings->nWidth;
    viewport.Height = settings->nHeight;

    pContext->RSSetViewports(1, &viewport);

    //////////////////////////////////////////////////////////////////////////

    D3D11_SAMPLER_DESC sampDesc = CD3D11_SAMPLER_DESC( CD3D11_DEFAULT() );
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    if (pDevice->CreateSamplerState( &sampDesc, &pFullscreenQuadSamplerState ) != S_OK)
      return false;

    //////////////////////////////////////////////////////////////////////////

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(D3D11_BUFFER_DESC) );
    cbDesc.ByteWidth = sizeof( pFullscreenQuadConstants );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ZeroMemory( &subData, sizeof(D3D11_SUBRESOURCE_DATA) );
    subData.pSysMem = &pFullscreenQuadConstants;

    if (pDevice->CreateBuffer( &cbDesc, &subData, &pFullscreenQuadConstantBuffer ) != S_OK)
      return false;

    //////////////////////////////////////////////////////////////////////////

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC( CD3D11_DEFAULT() );
    if (pDevice->CreateBlendState( &blendDesc, &pFullscreenQuadBlendState ) != S_OK)
      return false;

    D3D11_RASTERIZER_DESC rastDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT() );
    if (pDevice->CreateRasterizerState( &rastDesc, &pFullscreenQuadRasterizerState ) != S_OK)
      return false;

    return true;
  }

#define GUIQUADVB_SIZE (1024 * 6)

  char defaultGUIPixelShader[65536] = 
    "Texture2D tex;\n"
    "SamplerState smp;\n"
    "float4 main( float4 position : SV_POSITION, float4 Color: COLOR, float2 TexCoord : TEXCOORD0, float Factor : TEXCOORD1 ) : SV_TARGET\n"
    "{\n"
    "  float4 v4Texture = Color * tex.Sample(smp,TexCoord);//float4( Color.rgb, Color.a * .a );\n"
    "  float4 v4Color = Color;\n"
    "  return lerp( v4Texture, v4Color, Factor );\n"
    "}\n";
  char defaultGUIVertexShader[65536] = 
    "struct VS_INPUT_PP { float3 Pos : POSITION; float4 Color: COLOR; float2 TexCoord : TEXCOORD0; float Factor : TEXCOORD1; };\n"
    "struct VS_OUTPUT_PP { float4 Pos : SV_POSITION; float4 Color: COLOR; float2 TexCoord : TEXCOORD0; float Factor : TEXCOORD1; };\n"
    "\n"
    "cbuffer c { float4x4 matProj; float2 v2Offset; }\n"
    "\n"
    "VS_OUTPUT_PP main( VS_INPUT_PP In )\n"
    "{\n"
    "  VS_OUTPUT_PP Out;\n"
    "  Out.Pos = float4( In.Pos + float3(v2Offset,0), 1.0 );\n"
    "  Out.Pos = mul( Out.Pos, matProj );\n"
    "  Out.Color = In.Color;\n"
    "  Out.TexCoord = In.TexCoord;\n"
    "  Out.Factor = In.Factor;\n"
    "  return Out;\n"
    "}\n";

  ID3D11VertexShader * pGUIVertexShader = NULL;
  ID3D11PixelShader * pGUIPixelShader = NULL;
  ID3D11Buffer * pGUIQuadVB = NULL;
  ID3D11InputLayout * pGUIQuadLayout = NULL;
  ID3D11Buffer * pGUIConstantBuffer = NULL;
  ID3D11BlendState* pGUIBlendState = NULL;
  ID3D11RasterizerState * pGUIRasterizerState = NULL;
  float pGUIMatrix[ 4 * 4 + 16 ];

  void MatrixOrthoOffCenterLH(float * pout, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
  {
    ZeroMemory( pout, sizeof(float) * 4 * 4 );
    pout[0 + 0 * 4] = 2.0f / (r - l);
    pout[1 + 1 * 4] = 2.0f / (t - b);
    pout[2 + 2 * 4] = 1.0f / (zf -zn);
    pout[3 + 0 * 4] = -1.0f -2.0f *l / (r - l);
    pout[3 + 1 * 4] = 1.0f + 2.0f * t / (b - t);
    pout[3 + 2 * 4] = zn / (zn -zf);
    pout[3 + 3 * 4] = 1.0;
  }

  bool InitD3D11GUIRendering( RENDERER_SETTINGS * settings )
  {
    ID3DBlob * pCode = NULL;
    ID3DBlob * pErrors = NULL;
    if (D3DCompile( defaultGUIPixelShader, strlen(defaultGUIPixelShader), NULL, NULL, NULL, "main", "ps_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
    {
      printf("[Renderer] D3DCompile (PS) failed\n");
      return false;
    }

    if (pDevice->CreatePixelShader( pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pGUIPixelShader ) != S_OK)
    {
      printf("[Renderer] CreatePixelShader failed\n");
      return false;
    }

    if (D3DCompile( defaultGUIVertexShader, strlen(defaultGUIVertexShader), NULL, NULL, NULL, "main", "vs_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
    {
      printf("[Renderer] D3DCompile (VS) failed\n");
      return false;
    }

    if (pDevice->CreateVertexShader( pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pGUIVertexShader ) != S_OK)
    {
      printf("[Renderer] CreateVertexShader failed\n");
      return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

    desc.ByteWidth = sizeof(float) * 7 * GUIQUADVB_SIZE;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (pDevice->CreateBuffer(&desc, NULL, &pGUIQuadVB) != S_OK)
    {
      printf("[Renderer] CreateBuffer (VB) failed\n");
      return false;
    }

    static D3D11_INPUT_ELEMENT_DESC pGUIDesc[] =
    {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM , 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 4 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT      , 0, 6 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (pDevice->CreateInputLayout( pGUIDesc, 4, pCode->GetBufferPointer(), pCode->GetBufferSize(), &pGUIQuadLayout) != S_OK)
    {
      printf("[Renderer] CreateInputLayout failed\n");
      return false;
    }

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(D3D11_BUFFER_DESC) );
    cbDesc.ByteWidth = sizeof( float ) * 4 * 4 * 2;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    MatrixOrthoOffCenterLH( pGUIMatrix, 0, nWidth, nHeight, 0, -1.0f, 1.0f );
    
    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory( &subData, sizeof(D3D11_SUBRESOURCE_DATA) );
    subData.pSysMem = &pGUIMatrix;

    if (pDevice->CreateBuffer( &cbDesc, &subData, &pGUIConstantBuffer ) != S_OK)
    {
      printf("[Renderer] CreateBuffer (CB) failed\n");
      return false;
    }

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC( CD3D11_DEFAULT() );
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    if (pDevice->CreateBlendState( &blendDesc, &pGUIBlendState ) != S_OK)
      return false;

    D3D11_RASTERIZER_DESC rastDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT() );
    rastDesc.ScissorEnable = true;
    if (pDevice->CreateRasterizerState( &rastDesc, &pGUIRasterizerState ) != S_OK)
      return false;

    return true;
  }

  bool Open( RENDERER_SETTINGS * settings )
  {
    nWidth  = settings->nWidth;
    nHeight = settings->nHeight;

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

    if (!InitD3D11QuadRendering(settings))
    {
      printf("[Renderer] InitD3D11QuadRendering failed\n");
      return false;
    }

    if (!InitD3D11GUIRendering(settings))
    {
      printf("[Renderer] InitD3D11GUIRendering failed\n");
      return false;
    }

    return true;
  }

  void StartFrame()
  {
    MSG msg;
    if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) 
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }

    float f[4] = { 0.08f, 0.18f, 0.18f, 1.0f };
    pContext->ClearRenderTargetView(pRenderTarget, f);
  }
  void EndFrame()
  {
    pSwapChain->Present( bVsync ? 1 : 0, NULL );
  }
  bool WantsToQuit()
  {
    return !run;
  }
  void Close()
  {
    // TODO: a bunch of other crap needs to be deallocated here but i cant be arsed

    if (theShader) theShader->Release();
    if (pShaderReflection) pShaderReflection->Release();
    if (pVertexShader) pVertexShader->Release();

    if (pFullscreenQuadLayout) pFullscreenQuadLayout->Release();
    if (pFullscreenQuadVB) pFullscreenQuadVB->Release();
    if (pFullscreenQuadSamplerState) pFullscreenQuadSamplerState->Release();
    if (pFullscreenQuadConstantBuffer) pFullscreenQuadConstantBuffer->Release();
    if (pFullscreenQuadBlendState) pFullscreenQuadBlendState->Release();
    if (pFullscreenQuadRasterizerState) pFullscreenQuadRasterizerState->Release();

    if (pGUIRasterizerState) pGUIRasterizerState->Release();
    if (pGUIBlendState) pGUIBlendState->Release();
    if (pGUIConstantBuffer) pGUIConstantBuffer->Release();
    if (pGUIPixelShader) pGUIPixelShader->Release();
    if (pGUIVertexShader) pGUIVertexShader->Release();
    if (pGUIQuadLayout) pGUIQuadLayout->Release();
    if (pGUIQuadVB) pGUIQuadVB->Release();

    if (pRenderTarget) pRenderTarget->Release();
    if (pContext) pContext->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pDevice) pDevice->Release();
    if (hWnd) 
    {
      DestroyWindow(hWnd);
      UnregisterClass(_T("fwzwnd"),GetModuleHandle(NULL));
    }
  }

  void RenderFullscreenQuad()
  {
    float factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f, };
    pContext->VSSetShader( pVertexShader, NULL, NULL );
    pContext->PSSetShader( theShader, NULL, NULL );
    pContext->IASetInputLayout( pFullscreenQuadLayout );
    pContext->PSSetSamplers( 0, 1, &pFullscreenQuadSamplerState );
    pContext->PSSetConstantBuffers( 0, 1, &pFullscreenQuadConstantBuffer );
    pContext->OMSetBlendState( pFullscreenQuadBlendState, factor, 0xFFFFFFFF );
    pContext->RSSetState( pFullscreenQuadRasterizerState );

    ID3D11Buffer * buffers[] = { pFullscreenQuadVB };
    UINT stride[] = { sizeof(float) * 5 };
    UINT offset[] = { 0 };

    pContext->IASetVertexBuffers( 0, 1, buffers, stride, offset );
    pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    pContext->Draw( 4, 0 );
  }

  ID3D11ShaderReflectionConstantBuffer * pCBuf = NULL;
  bool ReloadShader( const char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    ID3DBlob * pCode = NULL;
    ID3DBlob * pErrors = NULL;
    if (D3DCompile( szShaderCode, nShaderCodeSize, NULL, NULL, NULL, "main", "ps_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
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

    if (pDevice->CreatePixelShader( pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &theShader ) != S_OK)
    {
      return false;
    }
    D3DReflect( pCode->GetBufferPointer(), pCode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection );
    pCBuf = pShaderReflection->GetConstantBufferByIndex(0);

    return true;
  }

  void __UpdateConstants()
  {
    D3D11_MAPPED_SUBRESOURCE subRes;
    pContext->Map( pFullscreenQuadConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subRes );
    CopyMemory( subRes.pData, &pFullscreenQuadConstants, sizeof(pFullscreenQuadConstants) );
    pContext->Unmap( pFullscreenQuadConstantBuffer, NULL );
  }

  void SetShaderConstant( const char * szConstName, float x )
  {
    ID3D11ShaderReflectionVariable * pCVar = pCBuf->GetVariableByName( szConstName );
    D3D11_SHADER_VARIABLE_DESC pDesc;
    if (pCVar->GetDesc( &pDesc ) != S_OK)
      return;
    
    ((float*)(((unsigned char*)&pFullscreenQuadConstants) + pDesc.StartOffset))[0] = x;

    __UpdateConstants();
  }

  void SetShaderConstant( const char * szConstName, float x, float y )
  {
    ID3D11ShaderReflectionVariable * pCVar = pCBuf->GetVariableByName(szConstName);
    D3D11_SHADER_VARIABLE_DESC pDesc;
    if (pCVar->GetDesc( &pDesc ) != S_OK)
      return;

    ((float*)(((unsigned char*)&pFullscreenQuadConstants) + pDesc.StartOffset))[0] = x;
    ((float*)(((unsigned char*)&pFullscreenQuadConstants) + pDesc.StartOffset))[1] = y;

    __UpdateConstants();
  }

  struct DX11Texture : public Texture
  {
    ID3D11Resource * pTexture;
    DXGI_FORMAT format;
    ID3D11ShaderResourceView * pResourceView;
  };

  void CreateResourceView( DX11Texture * tex )
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC) );
    desc.Format = tex->format;
    if (tex->type == TEXTURETYPE_1D)
    {
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
      desc.Texture1D.MostDetailedMip = 0;
      desc.Texture1D.MipLevels = 1;
    }
    else if (tex->type == TEXTURETYPE_2D)
    {
      desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MostDetailedMip = 0;
      desc.Texture2D.MipLevels = 1;
    }
    pDevice->CreateShaderResourceView( tex->pTexture, &desc, &tex->pResourceView );
  }

  int textureUnit = 0;

  Renderer::Texture * CreateRGBA8Texture()
  {
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof( D3D11_TEXTURE2D_DESC ) );
    desc.Width = nWidth;
    desc.Height = nHeight;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D * pTex = NULL;

    if ( pDevice->CreateTexture2D( &desc, NULL, &pTex ) != S_OK )
      return NULL;

    DX11Texture * tex = new DX11Texture();
    tex->width = nWidth;
    tex->height = nHeight;
    tex->pTexture = pTex;
    tex->type = TEXTURETYPE_2D;
    tex->format = desc.Format;
    CreateResourceView( tex );
    return tex;

  }

  Texture * CreateRGBA8TextureFromFile( const char * szFilename )
  {
    int comp = 0;
    int width = 0;
    int height = 0;

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    void * data = NULL;
    unsigned int pitch = 0;
    if ( stbi_is_hdr( szFilename ) )
    {
      data = stbi_loadf( szFilename, &width, &height, &comp, STBI_rgb_alpha );
      format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      pitch = width * sizeof( float ) * 4;
    }
    else
    {
      data = stbi_load( szFilename, &width, &height, &comp, STBI_rgb_alpha );
      pitch = width * sizeof( unsigned char ) * 4;
    }

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc,sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData,sizeof(D3D11_SUBRESOURCE_DATA));
    subData.pSysMem = data;
    subData.SysMemPitch = pitch;

    ID3D11Texture2D * pTex = NULL;

    if (pDevice->CreateTexture2D( &desc, &subData, &pTex ) != S_OK)
      return NULL;

    stbi_image_free(data);

    DX11Texture * tex = new DX11Texture();
    tex->width = width;
    tex->height = height;
    tex->pTexture = pTex;
    tex->type = TEXTURETYPE_2D;
    tex->format = desc.Format;
    CreateResourceView(tex);
    return tex;
  }

  Texture * Create1DR32Texture( int w )
  {
    D3D11_TEXTURE1D_DESC desc;
    ZeroMemory(&desc,sizeof(D3D11_TEXTURE1D_DESC));
    desc.Width = w;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture1D * pTex = NULL;

    if (pDevice->CreateTexture1D( &desc, NULL, &pTex ) != S_OK)
      return NULL;

    DX11Texture * tex = new DX11Texture();
    tex->width = w;
    tex->height = 1;
    tex->pTexture = pTex;
    tex->type = TEXTURETYPE_1D;
    tex->format = desc.Format;
    CreateResourceView(tex);
    return tex;
  }

  void SetShaderTexture( const char * szTextureName, Texture * tex )
  {
    D3D11_SHADER_INPUT_BIND_DESC desc;
    if (pShaderReflection->GetResourceBindingDescByName( szTextureName, &desc ) == S_OK)
    {
      DX11Texture * pTex = (DX11Texture *) tex;
      pContext->PSSetShaderResources( desc.BindPoint, 1, &pTex->pResourceView );
    }
  }

  bool UpdateR32Texture( Texture * tex, float * data )
  {
    ID3D11Texture1D * pTex = (ID3D11Texture1D *) ((DX11Texture *) tex)->pTexture;

    D3D11_MAPPED_SUBRESOURCE subRes;
    pContext->Map( pTex, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subRes );
    CopyMemory( subRes.pData, data, sizeof(float) * tex->width );
    pContext->Unmap( pTex, NULL );
    return true;
  }

  Texture * CreateA8TextureFromData( int w, int h, const unsigned char * data )
  {
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc,sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = w;
    desc.Height = h;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    unsigned int * p = new unsigned int[ w * h ];
    for (int i=0; i < w * h; i++)
      p[i] = (data[i] << 24) | 0xFFFFFF;
    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData,sizeof(D3D11_SUBRESOURCE_DATA));
    subData.pSysMem = p;
    subData.SysMemPitch = w * sizeof(unsigned int);

    ID3D11Texture2D * pTex = NULL;

    if (pDevice->CreateTexture2D( &desc, &subData, &pTex ) != S_OK)
      return NULL;

    delete[] p;

    DX11Texture * tex = new DX11Texture();
    tex->width = w;
    tex->height = h;
    tex->pTexture = pTex;
    tex->type = TEXTURETYPE_2D;
    tex->format = desc.Format;
    CreateResourceView(tex);
    return tex;
  }

  void ReleaseTexture( Texture * tex )
  {
    ((DX11Texture *)tex)->pResourceView->Release();
    ((DX11Texture *)tex)->pTexture->Release();
    delete tex;
  }

  void CopyBackbufferToTexture( Texture * tex )
  {
    ID3D11Resource * pTex = ( (DX11Texture *) tex )->pTexture;

    pContext->CopySubresourceRegion( pTex, 0, 0, 0, 0, pBackBuffer, 0, NULL );
  }

  //////////////////////////////////////////////////////////////////////////
  // text rendering

  int nDrawCallCount = 0;
  Texture * lastTexture = NULL;
  void StartTextRendering()
  {
    float factor[4] = { 1.0, 1.0, 1.0, 1.0 };
    pContext->VSSetShader( pGUIVertexShader, NULL, NULL );
    pContext->PSSetShader( pGUIPixelShader, NULL, NULL );
    pContext->IASetInputLayout( pGUIQuadLayout );
    pContext->VSSetConstantBuffers( 0, 1, &pGUIConstantBuffer );
    pContext->OMSetBlendState( pGUIBlendState, factor, 0xFFFFFFFF );
    pContext->RSSetState( pGUIRasterizerState );

    // Disable previous texture
    ID3D11ShaderResourceView * dummy[1] = { NULL };
    pContext->PSSetShaderResources( 0, 1, dummy );

    ID3D11Buffer * buffers[] = { pGUIQuadVB };
    UINT stride[] = { sizeof(float) * 7 };
    UINT offset[] = { 0 };

    pContext->IASetVertexBuffers( 0, 1, buffers, stride, offset );
    lastTexture = NULL;
    
    nDrawCallCount = 0;
  }

  int bufferPointer = 0;
  unsigned char buffer[GUIQUADVB_SIZE * sizeof(float) * 7];
  bool lastModeIsQuad = true;
  void __FlushRenderCache()
  {
    if (!bufferPointer) return;

    D3D11_MAPPED_SUBRESOURCE subRes;
    pContext->Map( pGUIQuadVB, 0, D3D11_MAP_WRITE_DISCARD, NULL, &subRes );
    CopyMemory( subRes.pData, buffer, bufferPointer * sizeof(float) * 7 );
    pContext->Unmap( pGUIQuadVB, 0 );

    if (lastModeIsQuad)
    {
      pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    }
    else
    {
      pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
    }
    pContext->Draw( bufferPointer, 0 );
    nDrawCallCount++;

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
        DX11Texture * pTex = (DX11Texture *) tex;
        pContext->PSSetShaderResources( 0, 1, &pTex->pResourceView );
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

    pGUIMatrix[ 16 + 0 ] = rect.left;
    pGUIMatrix[ 16 + 1 ] = rect.top;

    D3D11_MAPPED_SUBRESOURCE subRes;
    pContext->Map( pGUIConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subRes );
    CopyMemory( subRes.pData, &pGUIMatrix, sizeof(float) * 16 * 2 );
    pContext->Unmap( pGUIConstantBuffer, NULL );

    D3D11_RECT rc = { rect.left, rect.top, rect.right, rect.bottom };
    pContext->RSSetScissorRects( 1, &rc );
  }
  void EndTextRendering()
  {
    __FlushRenderCache();
  }

  //////////////////////////////////////////////////////////////////////////

  bool GrabFrame( void * pPixelBuffer )
  {
    if (!pFrameGrabTexture)
      return false;

    pContext->CopyResource( pFrameGrabTexture, pBackBuffer );

    D3D11_MAPPED_SUBRESOURCE resource;
    unsigned int nSubresource = D3D11CalcSubresource( 0, 0, 0 );
    if (pContext->Map( pFrameGrabTexture, nSubresource, D3D11_MAP_READ, 0, &resource ) != S_OK)
      return false;
    
    unsigned char* pSrc = (unsigned char*)resource.pData;
    unsigned char* pDst = (unsigned char*)pPixelBuffer;
    for( int i = 0; i < nHeight; i++ )
    {
      memcpy( pDst, pSrc, nWidth * 4 );
      pSrc += resource.RowPitch;
      pDst += nWidth * 4;
    }

    pContext->Unmap( pFrameGrabTexture, nSubresource );

    return true;
  }

}
