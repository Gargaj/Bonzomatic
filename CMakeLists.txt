cmake_minimum_required(VERSION 3.0)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
string(TIMESTAMP VERSION_PATCH "%Y%m%d")

project(Bonzomatic VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if (APPLE OR WIN32)
	set(BZC_EXE_NAME "Bonzomatic")
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

  ############################################################################
  # Windows
  if (WIN32)
    option(BONZOMATIC_64BIT "Compile for 64 bit target?" ON)
    if (CMAKE_GENERATOR MATCHES "64")
      set(BONZOMATIC_64BIT ON CACHE BOOL "Compile for 64 bit target?")
    else ()
      set(BONZOMATIC_64BIT OFF CACHE BOOL "Compile for 64 bit target?")
    endif ()
    option(BONZOMATIC_NDI "Enable NDI?" OFF)
    set(BONZOMATIC_WINDOWS_FLAVOR "GLFW" CACHE STRING "Windows renderer flavor selected at CMake configure time (DX11, DX9 or GLFW)")
    set_property(CACHE BONZOMATIC_WINDOWS_FLAVOR PROPERTY STRINGS DX11 DX9 GLFW)

    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE")
    #set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /VERBOSE")
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /VERBOSE")
    if (BONZOMATIC_64BIT)
      set(BZC_LINK_DIRS ${BZC_LINK_DIRS} $ENV{DXSDK_DIR}/lib/x64)
    else ()
      set(BZC_LINK_DIRS ${BZC_LINK_DIRS} $ENV{DXSDK_DIR}/lib/x86)
    endif ()

    if (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX11")
      set(PLATFORM_LIBS d3d11 d3dcompiler dxguid DXGI winmm shlwapi)
    elseif (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX9")
        set(PLATFORM_LIBS d3d9 d3dx9 winmm shlwapi)
    else ()
        set(PLATFORM_LIBS opengl32 glu32 winmm shlwapi)
    endif ()
  endif ()

  ############################################################################
  # MacOS
  if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES x86_64)
    set(CMAKE_FIND_FRAMEWORK LAST)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "No")
    set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "No")

    option(BONZOMATIC_TOUCHBAR "Compile with macOS TouchBar support (Xcode 9 or newer required)?" ON)

    set(GUI_TYPE MACOSX_BUNDLE)

    # Define some settings for the Bundle
    set(MACOSX_BUNDLE_BUNDLE_NAME ${BZC_EXE_NAME})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "${BZC_EXE_NAME}")
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(MACOSX_BUNDLE_INFO_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH},Copyright © 2020 The Bonzomatic Contributors")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2020 The Bonzomatic Contributors. All rights reserved.")
  
    set(BZC_PROJECT_SRCS ${GUI_TYPE} ${BZC_PROJECT_SRCS})
  
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(CARBON_FRAMEWORK Carbon)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    mark_as_advanced(COCOA_FRAMEWORK OPENGL_FRAMEWORK CARBON_FRAMEWORK COREAUDIO_FRAMEWORK AVFOUNDATION_FRAMEWORK)
    set(PLATFORM_LIBS ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${CARBON_FRAMEWORK} ${COREAUDIO_FRAMEWORK} ${AVFOUNDATION_FRAMEWORK})
  endif ()

##############################################################################
# Linux / BSD / Unix
else ()
	set(BZC_EXE_NAME "bonzomatic")
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

  option(BONZOMATIC_USE_SYSTEM_GLFW "Use the systems installed version of GLFW instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_GLEW "Use the systems installed version of GLEW instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_STB "Use the systems installed version of STB instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_MINIAUDIO "Use the systems installed version of Miniaudio instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_KISSFFT "Use the systems installed version of Kissfft instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_JSONXX "Use the systems installed version of Jsonxx instead of the vendored one (this build option is not supported by upstream)?" OFF)
  option(BONZOMATIC_USE_SYSTEM_SCINTILLA "Use the systems installed version of Scintilla instead of the vendored one (this build option is not supported by upstream)?" OFF)
  mark_as_advanced(BONZOMATIC_USE_SYSTEM_GLFW BONZOMATIC_USE_SYSTEM_GLEW BONZOMATIC_USE_SYSTEM_STB BONZOMATIC_USE_SYSTEM_MINIAUDIO BONZOMATIC_USE_SYSTEM_KISSFFT BONZOMATIC_USE_SYSTEM_JSONXX BONZOMATIC_USE_SYSTEM_SCINTILLA)
  
  if (BONZOMATIC_USE_SYSTEM_GLFW OR BONZOMATIC_USE_SYSTEM_GLEW OR BONZOMATIC_USE_SYSTEM_STB OR BONZOMATIC_USE_SYSTEM_MINIAUDIO OR BONZOMATIC_USE_SYSTEM_KISSFFT OR BONZOMATIC_USE_SYSTEM_JSONXX OR BONZOMATIC_USE_SYSTEM_SCINTILLA)
    find_package(PkgConfig REQUIRED)

    find_package(Threads REQUIRED)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} Threads::Threads)
  endif ()

  set(PLATFORM_LIBS GL asound fontconfig)
endif ()

##############################################################################
# Vendor Libs
##############################################################################
# Dont compile glfw and glew for windows dx targets
if (APPLE OR UNIX OR (WIN32 AND (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "GLFW")))
  ##############################################################################
  # GLFW
  # GLFW settings and project inclusion
  if (BONZOMATIC_USE_SYSTEM_GLFW)
    pkg_search_module(PKGCHK_GLFW REQUIRED glfw3)
    set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${PKGCHK_GLFW_INCLUDE_DIRS})
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PKGCHK_GLFW_LIBRARIES})
  else()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
    mark_as_advanced(BUILD_SHARED_LIBS GLFW_BUILD_EXAMPLES GLFW_BUILD_TESTS GLFW_BUILD_DOCS GLFW_INSTALL GLFW_VULKAN_STATIC)
    if (UNIX)
      set(GLFW_USE_OSMESA OFF CACHE BOOL "" FORCE)
      mark_as_advanced(GLFW_USE_OSMESA)
    endif()
    if (WIN32)
      set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
      mark_as_advanced(USE_MSVC_RUNTIME_LIBRARY_DLL)
    endif()
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw/)
    set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glfw/include)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} glfw ${GLFW_LIBRARIES})
  endif()

  ##############################################################################
  # GLEW
  if (BONZOMATIC_USE_SYSTEM_GLEW)
    pkg_search_module(PKGCHK_GLEW REQUIRED glew)
    set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${PKGCHK_GLEW_INCLUDE_DIRS})
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PKGCHK_GLEW_LIBRARIES})
  else ()
    set(GLEW_SRCS
      ${CMAKE_SOURCE_DIR}/external/glew/glew.c
    )
    add_library(bzc_glew STATIC ${GLEW_SRCS})
    target_include_directories(bzc_glew PUBLIC ${CMAKE_SOURCE_DIR}/external/glew)
    target_compile_definitions(bzc_glew PUBLIC -DGLEW_STATIC)
    if (MSVC)
      target_compile_options(bzc_glew PUBLIC "$<$<CONFIG:Release>:/MT>")
    endif ()
    set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glew)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_glew)
  endif()
else ()
  # for windows, use DirectX
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} $ENV{DXSDK_DIR}/Include)
endif ()

##############################################################################
# STB
if (BONZOMATIC_USE_SYSTEM_STB)
  find_path(STB_INCLUDE_DIR NAMES stb_image.h PATH_SUFFIXES stb REQUIRED)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${STB_INCLUDE_DIR})
else()
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/stb)
endif()

##############################################################################
# miniaudio
if (BONZOMATIC_USE_SYSTEM_MINIAUDIO)
  find_path(MNA_INCLUDE_DIR NAMES miniaudio.h PATH_SUFFIXES miniaudio REQUIRED)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${MNA_INCLUDE_DIR})
else()
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/miniaudio)
endif()

##############################################################################
# kiss_fft
if (BONZOMATIC_USE_SYSTEM_KISSFFT)
  pkg_search_module(PKGCHK_KISSFFT REQUIRED kissfft)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${PKGCHK_KISSFFT_INCLUDE_DIRS})
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PKGCHK_KISSFFT_LIBRARIES})
else ()
  set(KISSFFT_SRCS
    ${CMAKE_SOURCE_DIR}/external/kiss_fft/kiss_fft.c
    ${CMAKE_SOURCE_DIR}/external/kiss_fft/tools/kiss_fftr.c
  )
  add_library(bzc_kissfft STATIC ${KISSFFT_SRCS})
  target_include_directories(bzc_kissfft PUBLIC ${CMAKE_SOURCE_DIR}/external/kiss_fft ${CMAKE_SOURCE_DIR}/external/kiss_fft/tools)
  if (MSVC)
    target_compile_options(bzc_kissfft PUBLIC "$<$<CONFIG:Release>:/MT>")
  endif ()
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/kiss_fft)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/kiss_fft/tools)
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_kissfft)
endif()

##############################################################################
# JSONXX
if (BONZOMATIC_USE_SYSTEM_JSONXX)
  pkg_search_module(PKGCHK_JSONXX REQUIRED jsonxx)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${PKGCHK_JSONXX_INCLUDE_DIRS})
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PKGCHK_JSONXX_LIBRARIES})
else ()
  set(JSONXX_SRCS
    ${CMAKE_SOURCE_DIR}/external/jsonxx/jsonxx.cc
  )
  add_library(bzc_jsonxx STATIC ${JSONXX_SRCS})
  target_include_directories(bzc_jsonxx PUBLIC ${CMAKE_SOURCE_DIR}/external/jsonxx)
  if (MSVC)
    target_compile_options(bzc_jsonxx PUBLIC "$<$<CONFIG:Release>:/MT>")
  endif ()
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/jsonxx)
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_jsonxx)
endif()

##############################################################################
# NDI
if (WIN32 AND BONZOMATIC_NDI)
  if(DEFINED ENV{NDI_SDK_DIR}) 
    set(NDI_SDK_DIR "$ENV{NDI_SDK_DIR}")
  else()
    message(FATAL_ERROR "Could not find NDI SDK. The NDI_SDK_DIR environment variable must be set to the SDK path.")
  endif()

  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} "${NDI_SDK_DIR}/Include/")
  if (BONZOMATIC_64BIT)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} "${NDI_SDK_DIR}/Lib/x64/Processing.NDI.Lib.x64.lib")
  else ()
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} "${NDI_SDK_DIR}/Lib/x86/Processing.NDI.Lib.x86.lib")
  endif ()
endif ()

##############################################################################
# SCINTILLA
if (BONZOMATIC_USE_SYSTEM_SCINTILLA)
  pkg_search_module(PKGCHK_SCINTILLA REQUIRED scintilla)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${PKGCHK_SCINTILLA_INCLUDE_DIRS})
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PKGCHK_SCINTILLA_LIBRARIES})
else ()
  add_definitions(-DSCI_LEXER -DSCI_NAMESPACE)
  if (UNIX)
    add_definitions(-DGTK)
  endif ()
  set(SCINTILLA_SRCS
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCPP.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexOthers.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/Accessor.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/CharacterCategory.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/CharacterSet.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/LexerBase.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/LexerModule.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/LexerNoExceptions.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/LexerSimple.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/PropSetSimple.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/StyleContext.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib/WordList.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/AutoComplete.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/CallTip.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/CaseConvert.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/CaseFolder.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Catalogue.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/CellBuffer.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/CharClassify.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/ContractionState.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Decoration.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Document.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/EditModel.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Editor.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/EditView.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/ExternalLexer.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Indicator.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/KeyMap.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/LineMarker.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/MarginView.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/PerLine.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/PositionCache.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/RESearch.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/RunStyles.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/ScintillaBase.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Selection.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/Style.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/UniConversion.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/ViewStyle.cxx
    ${CMAKE_SOURCE_DIR}/external/scintilla/src/XPM.cxx
  )
  add_library(bzc_scintilla STATIC ${SCINTILLA_SRCS})
  target_include_directories(bzc_scintilla PUBLIC
    ${CMAKE_SOURCE_DIR}/external/scintilla/include
    ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib
    ${CMAKE_SOURCE_DIR}/external/scintilla/src
  )
  if (MSVC)
    target_compile_options(bzc_scintilla PUBLIC "$<$<CONFIG:Release>:/MT>")
  endif ()
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES}
      ${CMAKE_SOURCE_DIR}/external/scintilla/include
      ${CMAKE_SOURCE_DIR}/external/scintilla/lexlib
      ${CMAKE_SOURCE_DIR}/external/scintilla/src
  )
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_scintilla)
endif ()

##############################################################################
# BONZOMATIC
##############################################################################
file(GLOB BZC_PROJECT_SRCS
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/src/*.h
)
source_group("Bonzomatic" FILES ${BZC_PROJECT_SRCS})
if (APPLE)
  set(BZC_PLATFORM_SRCS
    ${CMAKE_SOURCE_DIR}/src/platform_glfw/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_common/MIDIDummy.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/Misc.mm
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Timer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/SetupDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/Clipboard.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/TouchBar.mm
  )
  source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})
  set(BZC_RESOURCES_DATA
    ${CMAKE_SOURCE_DIR}/data/macosx/icon.icns
  )
  source_group("Data" FILES ${BZC_RESOURCES_DATA})
  set_source_files_properties(${BZC_RESOURCES_DATA} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif (UNIX)
  set(BZC_PLATFORM_SRCS
    ${CMAKE_SOURCE_DIR}/src/platform_glfw/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/MIDI.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Misc.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/SetupDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Timer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Clipboard.cpp
  )
  source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})
elseif (WIN32)
  if (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX11")
    set(BZC_PLATFORM_SRCS
      ${CMAKE_SOURCE_DIR}/src/platform_w32_dx11/Renderer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Clipboard.cpp
      ${CMAKE_SOURCE_DIR}/data/windows/resource.h
    )
  elseif (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX9")
    set(BZC_PLATFORM_SRCS
      ${CMAKE_SOURCE_DIR}/src/platform_w32_dx9/Renderer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Clipboard.cpp
      ${CMAKE_SOURCE_DIR}/data/windows/resource.h
    )
  else ()
    set(BZC_PLATFORM_SRCS
      ${CMAKE_SOURCE_DIR}/src/platform_glfw/Renderer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
      ${CMAKE_SOURCE_DIR}/src/platform_w32_common/Clipboard.cpp
      ${CMAKE_SOURCE_DIR}/data/windows/resource.h
    )
  endif ()
  source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})

  set(BZC_RESOURCES_DATA
    ${CMAKE_SOURCE_DIR}/data/windows/icon.ico
    ${CMAKE_SOURCE_DIR}/data/windows/SetupDialog.rc
  )
  source_group("Data" FILES ${BZC_RESOURCES_DATA})
  set(BZC_PROJECT_INCLUDES ${CMAKE_SOURCE_DIR}/data/windows ${BZC_PROJECT_INCLUDES})
endif ()

if (WIN32 AND BONZOMATIC_NDI)
  set(BZC_CAPTURE_SRCS
    ${CMAKE_SOURCE_DIR}/src/capturing/Capture_NDI.cpp
  )
else ()
  set(BZC_CAPTURE_SRCS
    ${CMAKE_SOURCE_DIR}/src/capturing/Capture_Dummy.cpp
  )
endif ()
source_group("Bonzomatic\\Capture" FILES ${BZC_CAPTURE_SRCS})

set(BZC_PROJECT_SRCS ${BZC_PROJECT_SRCS} ${BZC_PLATFORM_SRCS} ${BZC_RESOURCES_DATA} ${BZC_CAPTURE_SRCS})

set(BZC_PROJECT_INCLUDES ${CMAKE_SOURCE_DIR}/src ${BZC_PROJECT_INCLUDES})

##############################################################################
# create the executable
link_directories(${BZC_LINK_DIRS})
add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS} ${PLATFORM_LIBS})

############################################################################
# Windows
if (WIN32)
  if (BONZOMATIC_NDI)
    target_compile_definitions(${BZC_EXE_NAME} PUBLIC -DBONZOMATIC_ENABLE_NDI)
  endif ()
  if (MSVC)
    set_target_properties(${BZC_EXE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    target_compile_options(${BZC_EXE_NAME} PUBLIC "$<$<CONFIG:Release>:/MT>")

    if (BONZOMATIC_NDI)
      if (BONZOMATIC_64BIT)
        add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NDI_SDK_DIR}/Bin/x64/Processing.NDI.Lib.x64.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
      else ()
        add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NDI_SDK_DIR}/Bin/x86/Processing.NDI.Lib.x86.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
      endif ()
    endif ()
  endif ()
############################################################################
# MacOS
elseif (APPLE)
  set_target_properties(${BZC_EXE_NAME} PROPERTIES MACOSX_BUNDLE ON MACOSX_BUNDLE_BUNDLE_NAME "Bonzomatic" MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/data/macosx/MacOSXBundleInfo.plist.in)
  if (BONZOMATIC_TOUCHBAR)
    target_compile_definitions(${BZC_EXE_NAME} PUBLIC -DBONZOMATIC_ENABLE_TOUCHBAR)
  endif ()
##############################################################################
# Linux / BSD / Unix
else ()
  target_compile_options(${BZC_EXE_NAME} PUBLIC -std=c++11)

  install(TARGETS ${BZC_EXE_NAME} RUNTIME DESTINATION bin)
  install(FILES "${CMAKE_SOURCE_DIR}/data/unix/bonzomatic.desktop"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
  install(FILES "${CMAKE_SOURCE_DIR}/data/unix/icon.png"
          RENAME "bonzomatic.png"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps")
endif ()
