#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <d3d11.h>
#include <D3Dcompiler.h>

#include "../Renderer.h"

#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>
#include "../external/scintilla/include/Scintilla.h"
#include <float.h>

#define DEVTYPE D3DDEVTYPE_HAL

const char * shaderKeyword =
  "BlendState break Buffer cbuffer class compile const continue DepthStencilState"
  " DepthStencilView discard do else extern false for GeometryShader if in inline"
  " inout interface namespace linear centroid nointerpolation noperspective sample"
  " out pass PixelShader precise RasterizerState RenderTargetView return register"
  " Sampler Sampler1D Sampler2D Sampler3D SamplerCUBE SamplerState"
  " SamplerComparisonState shared Stateblock StateblockState static struct switch"
  " tbuffer technique technique10 technique11 texture Texture1D Texture1DArray"
  " Texture2D Texture2DArray Texture2DMS Texture2DMSArray Texture3D TextureCube"
  " TextureCubeArray true typedef uniform VertexShader void volatile while";

const char * shaderType = 
  "AppendStructuredBuffer bool bool2 bool3 bool4 Buffer ByteAddressBuffer"
  " ConsumeStructuredBuffer double double2 double3 double4 float float1x1"
  " float1x2 float1x3 float1x4 float2 float2x1 float2x2 float2x3 float2x4"
  " float3 float3x1 float3x2 float3x3 float3x4 float4 float4x1 float4x2"
  " float4x3 float4x4 half half2 half3 half4 InputPatch int int2 int3 int4 line"
  " lineadj LineStream matrix OutputPatch point PointStream RWBuffer"
  " RWByteAddressBuffer RWStructuredBuffer RWTexture1D RWTexture1DArray"
  " RWTexture2D RWTexture2DArray RWTexture3D string StructuredBuffer Texture1D"
  " Texture1DArray Texture2D Texture2DArray Texture2DMS Texture2DMSArray"
  " Texture3D triangle triangleadj TriangleStream uint uint2 uint3 uint4 vector";

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
  char * defaultShaderFilename = "shader.dx11.hlsl";
  char defaultShader[65536] = 
    "Texture2D texFFT;\n"
    "Texture2D texNoise;\n"
    "Texture2D texChecker;\n"
    "Texture2D texTex1;\n"
    "Texture2D texTex2;\n"
    "Texture2D texTex3;\n"
    "Texture2D texTex4;\n"
    "SamplerState smp;\n"
    "\n"
    "cbuffer constants\n"
    "{\n"
    "  float fGlobalTime;\n"
    "  float2 v2Resolution;\n"
    "}\n"
    "\n"
    "float4 main( float4 position : SV_POSITION, float2 TexCoord : TEXCOORD ) : SV_TARGET\n"
    "{\n"
    "  float2 uv = TexCoord;\n"
    "  uv -= 0.5;\n"
    "  uv /= float2(v2Resolution.y / v2Resolution.x, 1);"
    "\n"
    "  float2 m;\n"
    "  m.x = atan(uv.x / uv.y) / 3.14;\n"
    "  m.y = 1 / length(uv) * .2;\n"
    "  float d = m.y;\n"
    "\n"
    "  float f = texFFT.Sample( smp, d ).r * 100;\n"
    "  m.x += sin( fGlobalTime ) * 0.1;\n"
    "  m.y += fGlobalTime * 0.25;\n"
    "\n"
    "  float4 t = texTex2.Sample( smp, m.xy ) * d; // or /d\n"
    "  return f + t;// + uv.xyxy * 0.5 * (sin( fGlobalTime ) + 1.5);\n"
    "}";
  char defaultVertexShader[65536] = 
    "struct VS_INPUT_PP { float3 Pos : POSITION; float2 TexCoord : TEXCOORD; };\n"
    "struct VS_OUTPUT_PP { float4 Pos : SV_POSITION; float2 TexCoord : TEXCOORD; };\n"
    "\n"
    "VS_OUTPUT_PP main( VS_INPUT_PP In )\n"
    "{\n"
    "  VS_OUTPUT_PP Out;\n"
    "  Out.Pos = float4( In.Pos, 1.0 );\n"
    "  Out.TexCoord = In.TexCoord;\n"
    "  return Out;\n"
    "}\n";

  bool run = true;

  IDXGISwapChain * pSwapChain = NULL;
  ID3D11Device * pDevice = NULL;
  ID3D11DeviceContext * pContext = NULL;
  ID3D11RenderTargetView * pRenderTarget = NULL;
  ID3D11VertexShader * pVertexShader = NULL;
  ID3D11PixelShader * theShader = NULL;
  ID3D11ShaderReflection * pShaderReflection = NULL;
  ID3D11SamplerState * pSamplerState = NULL;
  ID3D11Buffer * pConstantBuffer = NULL;

  typedef struct {
    float fGlobalTime;
    float v2Resolution[3]; // has to be 3 to fill 16byte boundary
  } VS_CONSTANT_BUFFER;

  VS_CONSTANT_BUFFER constants;
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

    case WM_ACTIVATE: 
      {
        if((LOWORD(wParam) & WA_ACTIVE) || (LOWORD(wParam) & WA_CLICKACTIVE)) 
        {
          //nGotFocus = 1;
          //bActive = true;
        } 
        else 
        {
          //nLostFocus = 1;
          //bActive = false;
        }
      } break;

    case WM_SIZE: 
      {
        switch (wParam) 
        {
        case SIZE_MINIMIZED: 
          {
            //nLostFocus = 1;
          } break;
        case SIZE_MAXIMIZED:
        case SIZE_RESTORED:
          {
            //nGotFocus = 1;
          } break;
        }
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

  bool InitDirect3D(RENDERER_SETTINGS * pSetup) 
  {
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hWnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = pSetup->windowMode != RENDERER_WINDOWMODE_FULLSCREEN;

    DWORD deviceCreationFlags = 0;
#ifdef _DEBUG
    //deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
      return false;

    ID3D11Texture2D * pBackBuffer = NULL;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

    pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTarget);
    pBackBuffer->Release();

    pContext->OMSetRenderTargets(1, &pRenderTarget, NULL);

    return true;
  }

#define GUIQUADVB_SIZE (128*6)

  ID3D11Buffer * pFullscreenQuadVB = NULL;
  ID3D11InputLayout * pFullscreenQuadLayout = NULL;

  bool Open( RENDERER_SETTINGS * settings )
  {
    if (!InitWindow(settings))
      return false;

    if (!InitDirect3D(settings))
      return false;

    ID3DBlob * pCode = NULL;
    ID3DBlob * pErrors = NULL;
    if (D3DCompile( defaultVertexShader, strlen(defaultVertexShader), NULL, NULL, NULL, "main", "vs_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
    {
      return false;
    }

    if (pDevice->CreateVertexShader( pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, &pVertexShader ) != S_OK)
    {
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
      return false;

    static D3D11_INPUT_ELEMENT_DESC pQuadDesc[] =
    {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    pDevice->CreateInputLayout( pQuadDesc, 2, pCode->GetBufferPointer(), pCode->GetBufferSize(), &pFullscreenQuadLayout);

    //////////////////////////////////////////////////////////////////////////

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width  = settings->nWidth;
    viewport.Height = settings->nHeight;

    pContext->RSSetViewports(1, &viewport);

    //////////////////////////////////////////////////////////////////////////

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc,sizeof(D3D11_SAMPLER_DESC));
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.MinLOD = -FLT_MAX;
    sampDesc.MaxLOD = FLT_MAX;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    if (pDevice->CreateSamplerState( &sampDesc, &pSamplerState ) != S_OK)
      return false;

    //////////////////////////////////////////////////////////////////////////

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(D3D11_BUFFER_DESC) );
    cbDesc.ByteWidth = sizeof( VS_CONSTANT_BUFFER );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ZeroMemory( &subData, sizeof(D3D11_SUBRESOURCE_DATA) );
    subData.pSysMem = &constants;

    if (pDevice->CreateBuffer( &cbDesc, &subData, &pConstantBuffer ) != S_OK)
      return false;

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
    pSwapChain->Present( NULL, NULL );
  }
  bool WantsToQuit()
  {
    return !run;
  }
  void Close()
  {
    if (theShader) theShader->Release();
    if (pShaderReflection) pShaderReflection->Release();
    if (pVertexShader) pVertexShader->Release();
    if (pFullscreenQuadLayout) pFullscreenQuadLayout->Release();
    if (pFullscreenQuadVB) pFullscreenQuadVB->Release();

    if (pRenderTarget) pRenderTarget->Release();
    if (pContext) pContext->Release();
    if (pSwapChain) pSwapChain->Release();
    if (pDevice) pDevice->Release();
    if (!hWnd) 
    {
      DestroyWindow(hWnd);
      UnregisterClass(_T("fwzwnd"),GetModuleHandle(NULL));
    }
  }

  void RenderFullscreenQuad()
  {
    pContext->VSSetShader( pVertexShader, NULL, NULL );
    pContext->PSSetShader( theShader, NULL, NULL );
    pContext->IASetInputLayout( pFullscreenQuadLayout );
    pContext->PSSetSamplers( 0, 1, &pSamplerState );
    pContext->PSSetConstantBuffers( 0, 1, &pConstantBuffer );

    ID3D11Buffer * buffers[] = { pFullscreenQuadVB };
    UINT stride[] = { sizeof(float) * 5 };
    UINT offset[] = { 0 };

    pContext->IASetVertexBuffers( 0, 1, buffers, stride, offset );
    pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    pContext->Draw( 4, 0 );
  }

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    ID3DBlob * pCode = NULL;
    ID3DBlob * pErrors = NULL;
    if (D3DCompile( szShaderCode, nShaderCodeSize, NULL, NULL, NULL, "main", "ps_4_0", NULL, NULL, &pCode, &pErrors ) != S_OK)
    {
      memset( szErrorBuffer, 0, nErrorBufferSize );
      strncpy( szErrorBuffer, (char*)pErrors->GetBufferPointer(), nErrorBufferSize - 1 );
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
    return true;
  }

  void __UpdateConstants()
  {
    D3D11_MAPPED_SUBRESOURCE subRes;
    pContext->Map( pConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &subRes );
    CopyMemory( subRes.pData, &constants, sizeof(VS_CONSTANT_BUFFER) );
    pContext->Unmap( pConstantBuffer, NULL );
  }

  void SetShaderConstant( char * szConstName, float x )
  {
    constants.fGlobalTime = x;
    __UpdateConstants();
  }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
    constants.v2Resolution[0] = x;
    constants.v2Resolution[1] = y;
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
  Texture * CreateRGBA8TextureFromFile( char * szFilename )
  {
    int comp = 0;
    int width = 0;
    int height = 0;
    unsigned char * c = stbi_load( szFilename, (int*)&width, (int*)&height, &comp, STBI_rgb_alpha );
    if (!c) return NULL;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc,sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData,sizeof(D3D11_SUBRESOURCE_DATA));
    subData.pSysMem = c;
    subData.SysMemPitch = width * sizeof(unsigned char) * 4;

    ID3D11Texture2D * pTex = NULL;

    if (pDevice->CreateTexture2D( &desc, &subData, &pTex ) != S_OK)
      return NULL;

    stbi_image_free(c);

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

  void SetShaderTexture( char * szTextureName, Texture * tex )
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

  Texture * CreateA8TextureFromData( int w, int h, unsigned char * data )
  {
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc,sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = w;
    desc.Height = h;
    desc.Format = DXGI_FORMAT_A8_UNORM;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subData;
    ZeroMemory(&subData,sizeof(D3D11_SUBRESOURCE_DATA));
    subData.pSysMem = data;
    subData.SysMemPitch = w * sizeof(unsigned char);

    ID3D11Texture2D * pTex = NULL;

    if (pDevice->CreateTexture2D( &desc, &subData, &pTex ) != S_OK)
      return NULL;

    DX11Texture * tex = new DX11Texture();
    tex->width = w;
    tex->height = h;
    tex->pTexture = pTex;
    tex->type = TEXTURETYPE_2D;
    tex->format = desc.Format;
    CreateResourceView(tex);
    return tex;
  }

  //////////////////////////////////////////////////////////////////////////
  // text rendering


  void StartTextRendering()
  {
  }

  void ReleaseTexture( Texture * tex )
  {
     ((DX11Texture *)tex)->pResourceView->Release();
     ((DX11Texture *)tex)->pTexture->Release();
     delete tex;
  }

  int bufferPointer = 0;
  unsigned char buffer[GUIQUADVB_SIZE * sizeof(float) * 6];
  bool lastModeIsQuad = true;
  void __FlushRenderCache()
  {
    if (!bufferPointer) return;

/*
    void * v = NULL;
    pGUIQuadVB->Lock( 0, bufferPointer * sizeof(float) * 6, &v, NULL );
    CopyMemory( v, buffer, bufferPointer * sizeof(float) * 6 );
    pGUIQuadVB->Unlock();

    pDevice->SetStreamSource( 0, pGUIQuadVB, 0, sizeof(float) * 6 ); 
    if (lastModeIsQuad)
      pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, bufferPointer / 3 );
    else
      pDevice->DrawPrimitive( D3DPT_LINELIST, 0, bufferPointer / 2 );
*/

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
//      pDevice->SetTexture( 0, tex ? ((DX9Texture *)tex)->pTexture : NULL );
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
    __WriteVertexToBuffer(d);
    __WriteVertexToBuffer(c);
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
/*
    __FlushRenderCache();
    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );
    mat._41 = rect.left;
    mat._42 = rect.top;
    pDevice->SetTransform( D3DTS_WORLD, &mat );

    RECT rc = { rect.left, rect.top, rect.right, rect.bottom };
    pDevice->SetScissorRect( &rc );
*/
  }
  void EndTextRendering()
  {
/*
    __FlushRenderCache();
    pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, false );
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );
*/
  }

}