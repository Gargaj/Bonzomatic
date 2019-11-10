#include <Windows.h>

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return;

    HGLOBAL hData = GlobalAlloc( GMEM_MOVEABLE, ( len + 1 ) * sizeof( char ) );
    if ( !hData ) {
      CloseClipboard();
      return;
    }

    WCHAR * pMem = (WCHAR*) GlobalLock( hData );
    if ( !pMem ) {
      GlobalFree( hData );
      CloseClipboard();
      return;
    }

    ZeroMemory( pMem, ( len + 1 ) * sizeof( char ) );
    CopyMemory( pMem, data, ( len + 1 ) * sizeof( char ) );

    GlobalUnlock( hData );

    EmptyClipboard();

    SetClipboardData( CF_TEXT, hData );

    CloseClipboard();
  }

  int GetContentsLength()
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return 0;

    HANDLE hData = GetClipboardData( CF_TEXT );
    if ( !hData ) {
      CloseClipboard();
      return 0;
    }

    const char * buffer = (const char*) GlobalLock( hData );
    if ( !buffer ) {
      CloseClipboard();
      return 0;
    }

    int n = strlen( buffer );

    GlobalUnlock( hData );

    CloseClipboard();

    return n;
  }

  void GetContents( char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return;

    HANDLE hData = GetClipboardData( CF_TEXT );
    if ( !hData ) {
      CloseClipboard();
      return;
    }

    const char * buffer = (const char*) GlobalLock( hData );
    if ( !buffer ) {
      CloseClipboard();
      return;
    }

    strncpy( data, buffer, len );

    GlobalUnlock( hData );

    CloseClipboard();
  }

}
