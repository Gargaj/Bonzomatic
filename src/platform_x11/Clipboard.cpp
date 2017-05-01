#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <string>
#include <string.h>

namespace Clipboard
{
  void Copy( const char * data, int len )
  {
    const std::string contents(data, len);
    GLFWwindow* window = glfwGetCurrentContext();
    glfwSetClipboardString(window, contents.c_str());
  }

  int GetContentsLength()
  {
    GLFWwindow* window = glfwGetCurrentContext();
    const char* contents = glfwGetClipboardString(window);
    return strlen(contents);
  }

  void GetContents( char * data, int len )
  {
    GLFWwindow* window = glfwGetCurrentContext();
    const char* contents = glfwGetClipboardString(window);
    strncpy(data, contents, len);
  }
}
