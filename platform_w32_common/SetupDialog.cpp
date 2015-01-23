#include <windows.h>
#include <tchar.h>
#include "../Renderer.h"
#include "resource.h"

#include <vector>
#include <algorithm>

class CSetupDialog;

CSetupDialog * pGlobal = NULL;

BOOL CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CSetupDialog
{
public:
  typedef struct {
    int nWidth;
    int nHeight;
  } RESOLUTION;

  std::vector<RESOLUTION> gaResolutions;
  HWND hWndSetupDialog;

  CSetupDialog(void)
  {
    hWndSetupDialog = NULL;
  }

  ~CSetupDialog(void)
  {
  }

  RENDERER_SETTINGS * setup;

  int __cdecl ResolutionSort(const void * a, const void * b)
  {
    RESOLUTION * aa = (RESOLUTION *)a;
    RESOLUTION * bb = (RESOLUTION *)b;
    if (aa->nWidth < bb->nWidth) return -1;
    if (aa->nWidth > bb->nWidth) return 1;
    if (aa->nHeight < bb->nHeight) return -1;
    if (aa->nHeight > bb->nHeight) return 1;
    return 0;
  }
  bool DialogProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
  {
    switch (uMsg) {
    case WM_INITDIALOG:
      {        
        int i = 0;
        while(1) {
          DEVMODE d;
          BOOL h = EnumDisplaySettings(NULL,i++,&d);
          if (!h) break;

          //if ((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight) continue;
          if (d.dmBitsPerPel != 32) continue;
          if (d.dmDisplayOrientation != DMDO_DEFAULT) continue;

          if (!gaResolutions.size() 
            || gaResolutions[ gaResolutions.size() - 1 ].nWidth != d.dmPelsWidth 
            || gaResolutions[ gaResolutions.size() - 1 ].nHeight != d.dmPelsHeight) 
          {
            RESOLUTION res;
            res.nWidth  = d.dmPelsWidth;
            res.nHeight = d.dmPelsHeight;
            gaResolutions.push_back(res);

          }
        }
        //std::sort(gaResolutions.begin(),gaResolutions.end(),&CSetupDialog::ResolutionSort);

        bool bFoundBest = false;
        for (i=0; i<gaResolutions.size() ; i++)
        {
          TCHAR s[50];
          _sntprintf(s,50,_T("%d * %d"),gaResolutions[i].nWidth,gaResolutions[i].nHeight);
          SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)s);

          if (gaResolutions[i].nWidth == GetSystemMetrics(SM_CXSCREEN) && gaResolutions[i].nHeight == GetSystemMetrics(SM_CYSCREEN))
          {
            SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_SETCURSEL, i, 0);
          }
        }

        if (setup->windowMode == RENDERER_WINDOWMODE_FULLSCREEN) {
          SendDlgItemMessage(hWnd, IDC_FULLSCREEN, BM_SETCHECK, 1, 1);
        }

        return true;
      } break;

    case WM_COMMAND:
      {
        switch( LOWORD(wParam) ) 
        {
        case IDOK: 
          {
            setup->nWidth  = gaResolutions[ SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0) ].nWidth;
            setup->nHeight = gaResolutions[ SendDlgItemMessage(hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0) ].nHeight;
            setup->windowMode = SendDlgItemMessage(hWnd, IDC_FULLSCREEN, BM_GETCHECK , 0, 0) ? RENDERER_WINDOWMODE_FULLSCREEN : RENDERER_WINDOWMODE_WINDOWED;
            EndDialog (hWnd, TRUE);
          } break;
        case IDCANCEL: 
          {
            EndDialog (hWnd, FALSE);
          } break;
        } 
      } break;
    }
    return false;
  }

  bool Open(HINSTANCE hInstance, HWND hWnd) {
    return DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SETUP),hWnd,DlgFunc,(LPARAM)this) != NULL;
  }
};

BOOL CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_INITDIALOG) {
    pGlobal = (CSetupDialog *)lParam; // todo: split to multiple hWnd-s! (if needed)
  }
  return pGlobal->DialogProcedure(hWnd,uMsg,wParam,lParam);
}

bool Renderer::OpenSetupDialog( RENDERER_SETTINGS * settings )
{
  CSetupDialog dlg;
  dlg.setup = settings;
  return dlg.Open( GetModuleHandle(NULL), NULL );
}