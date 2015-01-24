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
    "texture texTFFT; sampler1D texFFT = sampler_state { SRGBTexture = FALSE; Texture = <texTFFT>; }; \n"
    "texture texTNoise; sampler2D texNoise = sampler_state { SRGBTexture = TRUE; Texture = <texTNoise>; };\n"
    "texture texTChecker; sampler2D texChecker = sampler_state { SRGBTexture = TRUE; Texture = <texTChecker>; };\n"
    "texture texTTex1; sampler2D texTex1 = sampler_state { SRGBTexture = TRUE; Texture = <texTTex1>; };\n"
    "texture texTTex2; sampler2D texTex2 = sampler_state { SRGBTexture = TRUE; Texture = <texTTex2>; };\n"
    "texture texTTex3; sampler2D texTex3 = sampler_state { SRGBTexture = TRUE; Texture = <texTTex3>; };\n"
    "texture texTTex4; sampler2D texTex4 = sampler_state { SRGBTexture = TRUE; Texture = <texTTex4>; };\n"
    "\n"
    "float fGlobalTime;\n"
    "float2 v2Resolution;\n"
    "\n"
    "float4 main( float2 TexCoord : TEXCOORD0 ) : COLOR0\n"
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
  LPD3DXCONSTANTTABLE pConstantTable = NULL;
  LPDIRECT3DPIXELSHADER9 theShader = NULL;

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

    pDevice->SetRenderState( D3DRS_ZENABLE , D3DZB_FALSE );
    pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    for (int x=0; x<4; x++) 
    {
      pDevice->SetSamplerState( x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      pDevice->SetSamplerState( x, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }
    return 1;
  }

  LPDIRECT3DVERTEXBUFFER9 pFullscreenQuadVB = NULL;
  LPDIRECT3DVERTEXBUFFER9 pGUIQuadVB = NULL;
  LPDIRECT3DVERTEXDECLARATION9 pPostProcessVertexDecl = NULL;
  LPDIRECT3DVERTEXDECLARATION9 pGUIVertexDecl = NULL;

#define GUIQUADVB_SIZE (128*6)

  bool Open( RENDERER_SETTINGS * settings )
  {
    if (!InitWindow(settings))
      return false;

    if (!InitDirect3D(settings))
      return false;

    static D3DVERTEXELEMENT9 pFullscreenQuadElements[] = 
    {
      { 0, 0*sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 3*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      D3DDECL_END()
    };

    static float pQuad[] = {
      -1.0, -1.0,  0.0, 0.0, 0.0,
       1.0, -1.0,  0.0, 1.0, 0.0,
      -1.0,  1.0,  0.0, 0.0, 1.0,
       1.0,  1.0,  0.0, 1.0, 1.0,
    };

    pDevice->CreateVertexBuffer( 4 * 5 * sizeof(float), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pFullscreenQuadVB, NULL);
    void * v;
    pFullscreenQuadVB->Lock( 0, 4 * 5 * sizeof(float), &v, NULL );
    CopyMemory( v, pQuad, 4 * 5 * sizeof(float) );
    pFullscreenQuadVB->Unlock();

    pDevice->CreateVertexDeclaration( pFullscreenQuadElements, &pPostProcessVertexDecl );

    //////////////////////////////////////////////////////////////////////////

    static D3DVERTEXELEMENT9 pGUIQuadElements[] = 
    {
      { 0, 0*sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 3*sizeof(float), D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
      { 0, 4*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
      D3DDECL_END()
    };

    pDevice->CreateVertexBuffer( GUIQUADVB_SIZE * 6 * sizeof(float), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &pGUIQuadVB, NULL);
    //pDevice->CreateVertexDeclaration( pGUIQuadElements, &pGUIVertexDecl );

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
    pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

    D3DXMATRIX mat;
    D3DXMatrixIdentity( &mat );
    pDevice->SetTransform( D3DTS_VIEW, &mat );
    pDevice->SetTransform( D3DTS_WORLD, &mat );
    //D3DXMatrixOrthoOffCenterLH( (D3DXMATRIX*)&mat, -1.0f, -1.0, 1.0f, 1.0f, -1.0f, 1.0f );
    //D3DXMatrixOrthoOffCenterLH( &mat, -1.0, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f );
    pDevice->SetTransform( D3DTS_PROJECTION, &mat );

    pDevice->SetVertexShader( NULL );
    pDevice->SetPixelShader( theShader );
    
    pDevice->SetVertexDeclaration( pPostProcessVertexDecl );
    pDevice->SetStreamSource( 0, pFullscreenQuadVB, 0, sizeof(float) * 5 ); 
    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  }

  bool ReloadShader( char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize )
  {
    LPD3DXBUFFER pShader = NULL;
    LPD3DXBUFFER pErrors = NULL;

    if (D3DXCompileShader( szShaderCode, nShaderCodeSize, NULL, NULL, "main", "ps_2_0", NULL, &pShader, &pErrors, &pConstantTable ) != D3D_OK)
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

    if (pDevice->CreatePixelShader( (DWORD*)pShader->GetBufferPointer(), &theShader ) != D3D_OK)
    {
      return false;
    }

    return true;
  }

  void SetShaderConstant( char * szConstName, float x )
  {
    pConstantTable->SetFloat( pDevice, szConstName, x );
  }

  void SetShaderConstant( char * szConstName, float x, float y )
  {
    pConstantTable->SetVector( pDevice, szConstName, &D3DXVECTOR4(x, y, 0, 0) );
  }

  struct DX9Texture : public Texture
  {
    LPDIRECT3DTEXTURE9 pTexture;
  };

  int textureUnit = 0;
  Texture * CreateRGBA8TextureFromFile( char * szFilename )
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    D3DXIMAGE_INFO info;
    HRESULT h = D3DXCreateTextureFromFileExA(
      pDevice,
      szFilename,
      D3DX_DEFAULT,
      D3DX_DEFAULT,
      D3DX_DEFAULT,
      NULL,
      D3DFMT_FROM_FILE,
      D3DPOOL_DEFAULT,
      D3DX_FILTER_NONE,
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

  void SetShaderTexture( char * szTextureName, Texture * tex )
  {
    int idx = pConstantTable->GetSamplerIndex( szTextureName );
    if (idx >= 0)
      pDevice->SetTexture( idx, ((DX9Texture *)tex)->pTexture );
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

  Texture * CreateA8TextureFromData( int w, int h, unsigned char * data )
  {
    LPDIRECT3DTEXTURE9 pTex = NULL;
    pDevice->CreateTexture( w, h, 0, NULL, D3DFMT_A8, D3DPOOL_MANAGED, &pTex, NULL );

    if (!pTex)
      return NULL;

    D3DLOCKED_RECT rect;
    pTex->LockRect( 0, &rect, NULL, NULL );
    //memset( rect.pBits, 0, w * sizeof(float) );
    unsigned char * src = data;
    unsigned char * dst = (unsigned char *)rect.pBits;
    for (int i=0; i<h; i++)
    {
      memcpy( dst, src, w * sizeof(unsigned char) );
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

    pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
  }

  void ReleaseTexture( Texture * tex )
  {
    ((DX9Texture *)tex)->pTexture->Release();
    delete tex;
  }

  int bufferPointer = 0;
  unsigned char buffer[GUIQUADVB_SIZE * sizeof(float) * 6];
  bool lastModeIsQuad = true;
  void __FlushRenderCache()
  {
    if (!bufferPointer) return;

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
  void __WriteVertexToBuffer( Vertex & v )
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
    *(f++) = v.u;
    *(f++) = v.v;
    bufferPointer++;
  }
  Texture * lastTexture = NULL;
  void BindTexture( Texture * tex )
  {
    if (lastTexture != tex)
    {
      __FlushRenderCache();
      lastTexture = tex;
      pDevice->SetTexture( 0, tex ? ((DX9Texture *)tex)->pTexture : NULL );
    }
  }

  void RenderQuad( Vertex & a, Vertex & b, Vertex & c, Vertex & d )
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

  void RenderLine( Vertex & a, Vertex & b )
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

}