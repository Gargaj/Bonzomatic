#include <windows.h>
#include "..\external\scintilla\include\Platform.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

//////////////////////////////////////////////////////////////////////////
// Dynamic libraries

class DynamicLibraryImpl : public DynamicLibrary {
protected:
  HMODULE h;
public:
  explicit DynamicLibraryImpl(const char *modulePath)
{
    h = ::LoadLibraryA(modulePath);
  }

  virtual ~DynamicLibraryImpl()
{
    if (h != NULL)
      ::FreeLibrary(h);
  }

  // Use GetProcAddress to get a pointer to the relevant function.
  virtual Function FindFunction(const char *name)
{
    if (h != NULL)
{
      // C++ standard doesn't like casts betwen function pointers and void pointers so use a union
      union {
        FARPROC fp;
        Function f;
      } fnConv;
      fnConv.fp = ::GetProcAddress(h, name);
      return fnConv.f;
    } else {
      return NULL;
    }
  }

  virtual bool IsValid()
{
    return h != NULL;
  }
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath)
{
  return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
}
