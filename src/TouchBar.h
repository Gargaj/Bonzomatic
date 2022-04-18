//
//  TouchBar.h
//  Bonzomatic
//
//  Created by Nightfox on 13/5/18.
//
#ifdef BONZOMATIC_ENABLE_TOUCHBAR

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

void ShowTouchBar(GLFWwindow *window);

#endif
