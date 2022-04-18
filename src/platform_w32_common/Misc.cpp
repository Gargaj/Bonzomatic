#include <string>
#include <Windows.h>
#include <tchar.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include <map>

namespace Misc
{
  void PlatformStartup()
  {
  }

  void PlatformShutdown()
  {
  }

  std::map<std::string,std::string> keymaps;
  void InitKeymaps()
  {
    HKEY hk = NULL;
    RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", NULL, KEY_READ, &hk );

    LONG result = NULL;
    int idx = 0;
    while ( result != ERROR_NO_MORE_ITEMS )
    {
      char szKeyName[255];
      ZeroMemory(szKeyName,255);
      DWORD nKeyName = 255;
      result = RegEnumKeyExA( hk, idx, szKeyName, &nKeyName, NULL, NULL, NULL, NULL);
      if (result == ERROR_SUCCESS)
      {
        HKEY hkSub = NULL;
        RegOpenKeyExA( hk, szKeyName, NULL, KEY_READ, &hkSub );

        BYTE szValue[255];
        ZeroMemory(szValue,255);
        DWORD nValue = 255;
        DWORD type = REG_SZ;
        LONG resultKey = RegQueryValueExA( hkSub, "Layout Text", NULL, &type, szValue, &nValue );

        CharLowerA( szKeyName );
        keymaps[szKeyName] = (const char*)szValue;
      }
      idx++;
    }
  }

  void GetKeymapName( char * sz )
  {
    char szCode[KL_NAMELENGTH];
    ::GetKeyboardLayoutNameA(szCode);
    CharLowerA( szCode );
    strncpy( sz, keymaps.count(szCode) ? keymaps[szCode].c_str() : "<unknown>" ,255);
  }

  bool ExecuteCommand( const char * cmd, const char * param )
  {
    HINSTANCE hI = ShellExecute( NULL, NULL, cmd, param, NULL, SW_SHOW );
    return (int)hI >= 32;
  }

  bool FileExists(const char * path)
  {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
  }

  const char * GetDefaultFontPath()
  {
    char windowsPath[ MAX_PATH ];
    if ( SHGetFolderPath( NULL, CSIDL_WINDOWS, NULL, 0, windowsPath ) != S_OK )
    {
      return NULL;
    }
    const char* fontPaths[] = 
    {
      "Fonts\\cour.ttf",
      NULL
    };
    for (int i = 0; fontPaths[i]; ++i)
    {
      static char fullPath[ MAX_PATH ] = { 0 };
      PathCombineA( fullPath, windowsPath, fontPaths[ i ] );
      if (FileExists( fullPath ))
      {
        return fullPath;
      }
    }
    return NULL;
  }
}