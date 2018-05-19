#include <string>
#include <Windows.h>
#include <tchar.h>

#include <map>

namespace Misc
{
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
        keymaps.insert( std::make_pair( szKeyName, (char*)szValue ) );
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
  bool ExecuteCommand( char * cmd, char * param )
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
    const char* fontPaths[] = 
    {
      "c:\\Windows\\Fonts\\cour.ttf",
      NULL
    };
    for (int i = 0; fontPaths[i]; ++i)
    {
      if (FileExists(fontPaths[i]))
      {
        return fontPaths[i];
      }
    }
    return NULL;
  }
}