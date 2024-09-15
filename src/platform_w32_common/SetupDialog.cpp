#include <windows.h>
#ifdef __MINGW32__
#include <stdio.h>
#endif
#include <tchar.h>
#include "../Renderer.h"
#include "../FFT.h"

#include "../SetupDialog.h"

#include "resource.h"

#include <vector>
#include <algorithm>
#include <iostream>


namespace SetupDialog
{
//////////////////////////////////////////////////////////////////////////

class CSetupDialog;

CSetupDialog * pGlobal = NULL;

INT_PTR CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

class CSetupDialog
{
public:
  typedef struct 
  {
    int nWidth;
    int nHeight;
  } RESOLUTION;

  typedef struct 
  {
    bool bIsCapture;
    void * pDeviceID;
  } AUDIODEVICE;

  std::vector<RESOLUTION> gaResolutions;
  std::vector<AUDIODEVICE> gaAudioDevices;
  HWND hWndSetupDialog;

  CSetupDialog( void )
  {
    hWndSetupDialog = NULL;
  }

  ~CSetupDialog( void )
  {
  }

  SETTINGS * setup;

  int __cdecl ResolutionSort( const void * a, const void * b )
  {
    RESOLUTION * aa = (RESOLUTION *) a;
    RESOLUTION * bb = (RESOLUTION *) b;
    if ( aa->nWidth < bb->nWidth ) return -1;
    if ( aa->nWidth > bb->nWidth ) return 1;
    if ( aa->nHeight < bb->nHeight ) return -1;
    if ( aa->nHeight > bb->nHeight ) return 1;
    return 0;
  }
  void FFTDeviceEnum( const bool bIsCaptureDevice, const char * szDeviceName, void * pDeviceID )
  {
    TCHAR sz[ 512 ];
    _sntprintf( sz, 512, _T( "[%hs] %hs" ), bIsCaptureDevice ? "in" : "out", szDeviceName );
    SendDlgItemMessage( hWndSetupDialog, IDC_AUDIOSOURCE, CB_ADDSTRING, 0, (LPARAM) sz );

    if ( !pDeviceID )
    {
      if ( setup->sFFT.bUseRecordingDevice == bIsCaptureDevice )
      {
        SendDlgItemMessage( hWndSetupDialog, IDC_AUDIOSOURCE, CB_SETCURSEL, gaAudioDevices.size(), 0 );
      }
    }

    AUDIODEVICE audioDevice;
    audioDevice.bIsCapture = bIsCaptureDevice;
    audioDevice.pDeviceID = pDeviceID;
    gaAudioDevices.push_back( audioDevice );
  }
  static void FFTDeviceEnum( const bool bIsCaptureDevice, const char * szDeviceName, void * pDeviceID, void * pUserContext )
  {
    ( (CSetupDialog *) pUserContext )->FFTDeviceEnum( bIsCaptureDevice, szDeviceName, pDeviceID );
  }
  bool DialogProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

   


    switch ( uMsg ) 
    {
      case WM_INITDIALOG:
        {
          hWndSetupDialog = hWnd;

          int i = 0;
          while ( 1 ) 
          {
            DEVMODE d;
            BOOL h = EnumDisplaySettings( NULL, i++, &d );
            if ( !h ) break;

            //if ((d.dmPelsWidth * 9) / 16 != d.dmPelsHeight) continue;
            if ( d.dmBitsPerPel != 32 ) continue;
            if ( d.dmDisplayOrientation != DMDO_DEFAULT ) continue;

            if ( !gaResolutions.size()
              || gaResolutions[ gaResolutions.size() - 1 ].nWidth != d.dmPelsWidth
              || gaResolutions[ gaResolutions.size() - 1 ].nHeight != d.dmPelsHeight )
            {
              RESOLUTION res;
              res.nWidth = d.dmPelsWidth;
              res.nHeight = d.dmPelsHeight;
              gaResolutions.push_back( res );

            }
          }
          //std::sort(gaResolutions.begin(),gaResolutions.end(),&CSetupDialog::ResolutionSort);

          bool bFoundBest = false;
          for ( i = 0; i < gaResolutions.size(); i++ )
          {
            TCHAR s[ 50 ];
            _sntprintf( s, 50, _T( "%d * %d" ), gaResolutions[ i ].nWidth, gaResolutions[ i ].nHeight );
            SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM) s );


            if ( gaResolutions[ i ].nWidth == setup->sRenderer.nWidth && gaResolutions[ i ].nHeight == setup->sRenderer.nHeight )
            {
              SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_SETCURSEL, i, 0 );
              bFoundBest = true;
            }
            if ( !bFoundBest && gaResolutions[ i ].nWidth == GetSystemMetrics( SM_CXSCREEN ) && gaResolutions[ i ].nHeight == GetSystemMetrics( SM_CYSCREEN ) )
            {
              SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_SETCURSEL, i, 0 );
            }
          }
          
          TCHAR s[50];
          _sntprintf(s, 50, _T("SENDER"));
          SendDlgItemMessage(hWnd, IDC_NETWORK_MODE, CB_ADDSTRING, 0, (LPARAM)s);
          _sntprintf(s, 50, _T("GRABBER"));
          SendDlgItemMessage(hWnd, IDC_NETWORK_MODE, CB_ADDSTRING, 0, (LPARAM)s);
          SendDlgItemMessage(hWnd, IDC_NETWORK_MODE, CB_SETCURSEL, 0, 0);
          if ( setup->sRenderer.windowMode == Renderer::WINDOWMODE_FULLSCREEN ) 
          {
            SendDlgItemMessage( hWnd, IDC_FULLSCREEN, BM_SETCHECK, 1, 1 );
          }
          if ( setup->sRenderer.bVsync ) 
          {
            SendDlgItemMessage( hWnd, IDC_VSYNC, BM_SETCHECK, 1, 1 );
          }

          std::string ServerName;
          std::string RoomName;
          std::string NickName;
         // Network_Break_URL(network->ServerURL, ServerName, RoomName, NickName);

          SetDlgItemText(hWnd, IDC_NETWORK_SERVER, "ws://drone.");
          SetDlgItemText(hWnd, IDC_NETWORK_ROOMNAME, "roomname");
          SetDlgItemText(hWnd, IDC_NETWORK_NICKNAME,  "handle");

          FFT::EnumerateDevices( FFTDeviceEnum, this );

          return true;
        } break;

      case WM_COMMAND:
        {

          switch ( LOWORD( wParam ) )
          {
            case IDOK:
              {
                setup->sRenderer.nWidth = gaResolutions[ SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0 ) ].nWidth;
                setup->sRenderer.nHeight = gaResolutions[ SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0 ) ].nHeight;
                setup->sRenderer.windowMode = SendDlgItemMessage( hWnd, IDC_FULLSCREEN, BM_GETCHECK, 0, 0 ) ? Renderer::WINDOWMODE_FULLSCREEN : Renderer::WINDOWMODE_WINDOWED;
                setup->sRenderer.bVsync = SendDlgItemMessage( hWnd, IDC_VSYNC, BM_GETCHECK, 0, 0 ) > 0;

                setup->sFFT.bUseRecordingDevice = gaAudioDevices[ SendDlgItemMessage( hWnd, IDC_AUDIOSOURCE, CB_GETCURSEL, 0, 0 ) ].bIsCapture;
                setup->sFFT.pDeviceID = gaAudioDevices[ SendDlgItemMessage( hWnd, IDC_AUDIOSOURCE, CB_GETCURSEL, 0, 0 ) ].pDeviceID;

                int ServerLen = SendDlgItemMessage(hWnd, IDC_NETWORK_SERVER, WM_GETTEXTLENGTH, 0, 0);
                char ServerName[512];
                GetDlgItemText(hWnd, IDC_NETWORK_SERVER, ServerName, min(ServerLen + 1, 511));

                int RoomLen = SendDlgItemMessage(hWnd, IDC_NETWORK_ROOMNAME, WM_GETTEXTLENGTH, 0, 0);
                char RoomName[512];
                GetDlgItemText(hWnd, IDC_NETWORK_ROOMNAME, RoomName, min(RoomLen + 1, 511));

                int NickLen = SendDlgItemMessage(hWnd, IDC_NETWORK_NICKNAME, WM_GETTEXTLENGTH, 0, 0);
                char NickName[512];
                GetDlgItemText(hWnd, IDC_NETWORK_NICKNAME, NickName, min(NickLen + 1, 511));
                EndDialog( hWnd, TRUE );
              } break;
            case IDC_NETWORK: // Combo Box Click 
              {         
   
                if (SendDlgItemMessage(hWnd, IDC_NETWORK, BM_GETCHECK, 0, 0)) {
                   // Activate 

                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_MODE), TRUE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_NICKNAME), TRUE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_ROOMNAME), TRUE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_SERVER), TRUE);
;                }
                else {
                  // Desactivate

                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_MODE), FALSE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_NICKNAME), FALSE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_ROOMNAME), FALSE);
                  EnableWindow(GetDlgItem(hWnd, IDC_NETWORK_SERVER), FALSE);
                }

              } break;
            case IDCANCEL:
              {
                EndDialog( hWnd, FALSE );
              } break;
          }
        } break;
    }
    return false;
  }

  bool Open( HINSTANCE hInstance, HWND hWnd ) 
  {
    return DialogBoxParam( hInstance, MAKEINTRESOURCE( IDD_SETUP ), hWnd, DlgFunc, (LPARAM) this ) != NULL;
  }
};

INT_PTR CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  if ( uMsg == WM_INITDIALOG ) 
  {
    pGlobal = (CSetupDialog *) lParam; // todo: split to multiple hWnd-s! (if needed)
  }
  return pGlobal->DialogProcedure( hWnd, uMsg, wParam, lParam );
}

bool Open( SetupDialog::SETTINGS * settings)
{
  CSetupDialog dlg;
  dlg.setup = settings;
  return dlg.Open( GetModuleHandle( NULL ), NULL );
}

//////////////////////////////////////////////////////////////////////////
}