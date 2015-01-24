#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../Renderer.h"

#define STBI_HEADER_FILE_ONLY
#include <stb_image.c>
#include "../external/scintilla/include/Scintilla.h"

#define DEVTYPE D3DDEVTYPE_HAL

namespace Renderer
{
  char defaultShader[65536] = 
    "float2 v2Resolution\n"
    "texture1D texFFT;\n"
    "texture2D texNoise;\n"
    "texture2D texChecker;\n"
    "texture2D texTex1;\n"
    "texture2D texTex2;\n"
    "\n"
    "float4 ps_main( float2 TexCoord : TEXCOORD0 )\n"
    "{\n"
    "  float2 uv = TexCoord;\n"
    "  uv -= 0.5;\n"
    "\n"
    "  float2 m;\n"
    "  m.x = atan(uv.x / uv.y) / 3.14;\n"
    "  m.y = 1 / length(uv) * .2;\n"
    "  float d = m.y;\n"
    "\n"
    "  float f = tex1D( texFFT, d ).r * 100;\n"
    "  m.x += sin( fGlobalTime ) * 0.1;\n"
    "  m.y += fGlobalTime * 0.25;\n"
    "\n"
    "  float4 t = tex2D( texTex2, m.xy ) * d; // or /d\n"
    "  return f + t;// + uv.xyxy * 0.5 * (sin( fGlobalTime ) + 1.5);\n"
    "}";

  bool run = true;

  LPDIRECT3D9 pD3D = NULL;
  LPDIRECT3DDEVICE9 pDevice = NULL;

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
        switch( wParam ) 
        {
        case VK_ESCAPE: 
          {
            run = false;
          } break;
        }
        //pKeys[wParam] = 1;
      } break;

    case WM_KEYUP: 
      {
        //pKeys[wParam] = 0;
      } break;

    case WM_LBUTTONDOWN: 
      {
        //nLeftMouse = 1;
      } break;

    case WM_LBUTTONUP: 
      {
        //nLeftMouse = 0;
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

    hWnd = CreateWindowEx(wExStyle,_T("fwzwnd"),_T("ümlaüt design"),wStyle,
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
    pD3D=Direct3DCreate9(D3D9b_SDK_VERSION);
    if (!pD3D) return false;

    nWidth  = pSetup->nWidth;
    nHeight = pSetup->nHeight;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp,sizeof(d3dpp));

    d3dpp.SwapEffect     = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow  = hWnd;
    d3dpp.Windowed       = pSetup->windowMode != RENDERER_WINDOWMODE_FULLSCREEN;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    d3dpp.BackBufferCount  = 1;

    d3dpp.BackBufferWidth  = pSetup->nWidth;
    d3dpp.BackBufferHeight = pSetup->nHeight;

    D3DDISPLAYMODE d3ddm;
    if( FAILED( pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
      return false;

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
      return false;

    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;

    static D3DFORMAT pDepthFormats[] = {
      D3DFMT_D16,
      D3DFMT_D24X8,
      D3DFMT_D32,
      D3DFMT_UNKNOWN
    };
    for (int i=0; pDepthFormats[i]!=D3DFMT_UNKNOWN; i++)
      if ( SUCCEEDED(pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, DEVTYPE, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, pDepthFormats[i] )) )
        d3dpp.AutoDepthStencilFormat = pDepthFormats[i];

    if (d3dpp.AutoDepthStencilFormat == D3DFMT_UNKNOWN) 
      return false;

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
        return false;
      }
    }

    pDevice->SetRenderState(D3DRS_ZENABLE , D3DZB_TRUE);
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    for (int x=0; x<4; x++) 
    {
      pDevice->SetSamplerState( x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }
    return 1;
  }

  bool Open( RENDERER_SETTINGS * settings )
  {
    if (!InitWindow(settings))
      return false;

    if (!InitDirect3D(settings))
      return false;

    return true;
  }

  void StartFrame()
  {
    MSG msg;
    if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }

    pDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER , 0xFF808080, 1.0f, 0 );
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
    if (pDevice) pDevice->Release();
    if (pD3D) pD3D->Release();
    if (!hWnd) 
    {
      DestroyWindow(hWnd);
      UnregisterClass(_T("fwzwnd"),GetModuleHandle(NULL));
    }
  }

  void RenderFullscreenQuad()
  {
  }

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    return true;
  }

  void SetShaderConstant( char * szConstName, float x )
  {
  }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
  }

  struct DX9Texture : public Texture
  {
    LPDIRECT3DTEXTURE9 pTexture;
  };

  int textureUnit = 0;
  Texture * CreateRGBA8TextureFromFile( char * szFilename )
  {
    DX9Texture * tex = new DX9Texture();
    tex->width = 0;
    tex->height = 0;
    tex->type = TEXTURETYPE_2D;
    return tex;
  }

  Texture * Create1DR32Texture( int w )
  {
    DX9Texture * tex = new DX9Texture();
    tex->width = w;
    tex->height = 1;
    tex->type = TEXTURETYPE_1D;
    return tex;
  }

  void SetShaderTexture( char * szTextureName, Texture * tex )
  {
  }

  bool UpdateR32Texture( Texture * tex, float * data )
  {
    return true;
  }

  void StartTextRendering()
  {
    pDevice->SetVertexShader( NULL );
    pDevice->SetPixelShader( NULL );
  }
  void SetTextRenderingViewport( Scintilla::PRectangle rect )
  {
  }
  void EndTextRendering()
  {
  }

  Texture * CreateA8TextureFromData( int w, int h, unsigned char * data )
  {
    DX9Texture * tex = new DX9Texture();
    tex->width = w;
    tex->height = 1;
    tex->type = TEXTURETYPE_1D;
    return tex;
  }

  void ReleaseTexture( Texture * tex )
  {
//     ((DX9Texture *)tex)->pTexture->Release();
//     delete tex;
  }

  void BindTexture( Texture * tex )
  {
//     pDevice->SetTexture( 0, ((DX9Texture *)tex)->pTexture );
  }

  void RenderQuad( Vertex & a, Vertex & b, Vertex & c, Vertex & d )
  {
  }

  void RenderLine( Vertex & a, Vertex & b )
  {
  }

}