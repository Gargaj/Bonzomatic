#include <iostream>

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    std::cerr << __FUNCTION__ << " NOT IMPLEMENTED" << std::endl;
  }

  int GetContentsLength()
  {
    std::cerr << __FUNCTION__ << " NOT IMPLEMENTED" << std::endl;
    return 0;
  }

  void GetContents( char * data, int len )
  {
    std::cerr << __FUNCTION__ << " NOT IMPLEMENTED" << std::endl;
    data[0] = 0;
  }

}
