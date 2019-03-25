#include <Windows.h>

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return;

    EmptyClipboard();

    HGLOBAL h = GlobalAlloc( GMEM_MOVEABLE, ( len + 1 ) * sizeof( char ) );
    WCHAR * pMem = (WCHAR*) GlobalLock( h );

    ZeroMemory( pMem, ( len + 1 ) * sizeof( char ) );
    CopyMemory( pMem, data, ( len + 1 ) * sizeof( char ) );

    GlobalUnlock( h );
    SetClipboardData( CF_TEXT, h );

    CloseClipboard();
  }

  int GetContentsLength()
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return 0;

    HANDLE hData = GetClipboardData( CF_TEXT );
    const char * buffer = (const char*) GlobalLock( hData );
    int n = strlen( buffer );

    CloseClipboard();

    return n;
  }

  void GetContents( char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if ( !::OpenClipboard( hWnd ) )
      return;

    HANDLE hData = GetClipboardData( CF_TEXT );
    const char * buffer = (const char*) GlobalLock( hData );
    strncpy( data, buffer, len );

    CloseClipboard();
  }

}
