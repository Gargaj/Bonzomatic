#include <Windows.h>

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if (!::OpenClipboard( hWnd ))
      return;

    EmptyClipboard();

    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE,(len + 1) * sizeof(WCHAR));
    WCHAR * pMem = (WCHAR*)GlobalLock(h);

    ZeroMemory( pMem, (len + 1) * sizeof(WCHAR) );
    CopyMemory( pMem, data, (len + 1) * sizeof(WCHAR) );

    GlobalUnlock(h);
    SetClipboardData(CF_TEXT,h);

    CloseClipboard();
  }

  int GetContentsLength()
  {
    HWND hWnd = GetForegroundWindow();
    if (!::OpenClipboard( hWnd ))
      return 0;

    HANDLE hData = GetClipboardData(CF_TEXT);
    char * buffer = (char*)GlobalLock( hData );
    int n = strlen(buffer);

    CloseClipboard();

    return n;
  }

  void GetContents( char * data, int len )
  {
    HWND hWnd = GetForegroundWindow();
    if (!::OpenClipboard( hWnd ))
      return;

    HANDLE hData = GetClipboardData(CF_TEXT);
    char * buffer = (char*)GlobalLock( hData );
    strncpy( data, buffer, len );

    CloseClipboard();
  }

}
