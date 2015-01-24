#include <iostream>
#include "../external/scintilla/include/Platform.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

//////////////////////////////////////////////////////////////////////////
// Dynamic libraries

class DynamicLibraryImpl : public DynamicLibrary {
public:
  explicit DynamicLibraryImpl(const char *modulePath)
  {
    std::cerr << __FUNCTION__ << "NOT IMPLEMENTED";
  }

  virtual ~DynamicLibraryImpl()
  {
    std::cerr << __FUNCTION__ << "NOT IMPLEMENTED";
  }

  // Use GetProcAddress to get a pointer to the relevant function.
  virtual Function FindFunction(const char *name)
  {
    std::cerr << __FUNCTION__ << "NOT IMPLEMENTED";
    return 0;
  }

  virtual bool IsValid()
  {
    std::cerr << __FUNCTION__ << "NOT IMPLEMENTED";
    return false;
  }
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath)
{
  return static_cast<DynamicLibrary *>(new DynamicLibraryImpl(modulePath));
}
