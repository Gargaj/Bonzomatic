#include <iostream>

void copy(const char *data) {
  std::shared_ptr<FILE> pipe(popen("pbcopy", "w"), pclose);
  if (!pipe)
    return;
  fputs(data, pipe.get());
}

std::string paste() {
  char buffer[128];
  std::string result = "";
  std::shared_ptr<FILE> pipe(popen("pbpaste", "r"), pclose);
  if (!pipe)
    return result;
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    char *buf = new char[len + 1];
    memcpy(buf, data, len);
    buf[len] = 0;
    copy(buf);
  }

  int GetContentsLength()
  {
    return paste().size();
  }

  void GetContents( char * data, int len )
  {
    auto contents = paste();
    memcpy(data, contents.c_str(), len);
  }

}
