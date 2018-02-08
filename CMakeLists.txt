project(Bonzomatic)

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(BZC_EXE_NAME "Bonzomatic")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")

option(BONZOMATIC_64BIT "Compile for 64 bit target?" ON)
if (MSVC)
  if (CMAKE_GENERATOR MATCHES "64")
    set(BONZOMATIC_64BIT ON CACHE BOOL "Compile for 64 bit target?")
  else ()
    set(BONZOMATIC_64BIT OFF CACHE BOOL "Compile for 64 bit target?")
  endif ()
else ()
  set(BONZOMATIC_64BIT ON CACHE BOOL "Compile for 64 bit target?")
endif ()

option(BONZOMATIC_NDI "Enable NDI?" OFF)

set(BONZOMATIC_WINDOWS_FLAVOR "DX11" CACHE STRING "Windows renderer flavor selected at CMake configure time (DX11, DX9 or GLFW)")
set_property(CACHE BONZOMATIC_WINDOWS_FLAVOR PROPERTY STRINGS DX11 DX9 GLFW) 

if (APPLE)
  set(CMAKE_FIND_FRAMEWORK LAST)
endif ()

add_definitions(-DSCI_LEXER -DSCI_NAMESPACE)
if (UNIX)
  add_definitions(-DGTK)
endif ()

if (MSVC)
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE")
  #set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /VERBOSE")
  #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /VERBOSE")
  if (BONZOMATIC_64BIT)
    set(BZC_LINK_DIRS ${BZC_LINK_DIRS} $ENV{DXSDK_DIR}/lib/x64)
  else ()
    set(BZC_LINK_DIRS ${BZC_LINK_DIRS} $ENV{DXSDK_DIR}/lib/x86)
  endif ()
endif ()

if (APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "No")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "No")
endif ()

# Dont compile glfw and glew for windows dx targets
if (APPLE OR UNIX OR (MSVC AND (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "GLFW")))
  ##############################################################################
  # GLFW
  # GLFW settings and project inclusion
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  mark_as_advanced(GLFW_BUILD_EXAMPLES GLFW_BUILD_TESTS GLFW_BUILD_DOCS GLFW_INSTALL)
  if (APPLE)
    set(GLFW_USE_CHDIR OFF CACHE BOOL "" FORCE)
    set(GLFW_USE_RETINA OFF CACHE BOOL "" FORCE)
    set(GLFW_USE_MENUBAR ON CACHE BOOL "" FORCE)
    mark_as_advanced(GLFW_USE_CHDIR GLFW_USE_RETINA GLFW_USE_MENUBAR)
  elseif (MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
    mark_as_advanced(USE_MSVC_RUNTIME_LIBRARY_DLL)
  endif()
  add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw/)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glfw/include)
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} glfw ${GLFW_LIBRARIES})

  ##############################################################################
  # GLEW
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
else ()
  # for windows, use DirectX
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} $ENV{DXSDK_DIR}/Include)
endif ()

##############################################################################
# STB
set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES}
  ${CMAKE_SOURCE_DIR}/external/stb
)

##############################################################################
# BASS
set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/bass)
if (APPLE)
  set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/libbass.dylib)
elseif (UNIX)
  if (BONZOMATIC_64BIT)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/x64/libbass.so)
  else ()
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/x86/libbass.so)
  endif ()
elseif (MSVC)
  if (BONZOMATIC_64BIT)
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/x64/bass.lib)
  else ()
    set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/x86/bass.lib)
  endif ()
endif ()

##############################################################################
# JSONXX
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

##############################################################################
# NDI
if (MSVC AND BONZOMATIC_NDI)
  set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} "c:/Program Files/NewTek/NewTek NDI SDK/Include/")
  if (BONZOMATIC_64BIT)
    set(BZC_LINK_DIRS ${BZC_LINK_DIRS} "c:/Program Files/NewTek/NewTek NDI SDK/Lib/x64/")
  else ()
    set(BZC_LINK_DIRS ${BZC_LINK_DIRS} "c:/Program Files/NewTek/NewTek NDI SDK/Lib/x86/")
  endif ()
endif ()

##############################################################################
# SCINTILLA
set(SCINTILLA_SRCS
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexA68k.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAbaqus.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAda.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAPDL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAsm.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAsn1.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexASY.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAU3.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAVE.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexAVS.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexBaan.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexBash.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexBasic.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexBibTeX.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexBullant.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCaml.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCLW.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCmake.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCOBOL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCoffeeScript.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexConf.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCPP.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCrontab.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCsound.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexCSS.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexD.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexDMAP.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexDMIS.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexECL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexEiffel.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexErlang.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexEScript.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexFlagship.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexForth.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexFortran.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexGAP.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexGui4Cli.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexHaskell.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexHTML.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexInno.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexKix.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexKVIrc.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexLaTeX.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexLisp.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexLout.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexLua.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMagik.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMarkdown.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMatlab.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMetapost.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMMIXAL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexModula.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMPT.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMSSQL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexMySQL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexNimrod.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexNsis.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexOpal.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexOScript.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexOthers.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPascal.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPB.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPerl.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPLM.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPO.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPOV.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPowerPro.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPowerShell.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexProgress.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPS.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexPython.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexR.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexRebol.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexRegistry.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexRuby.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexRust.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexScriptol.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSmalltalk.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSML.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSorcus.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSpecman.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSpice.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSQL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexSTTXT.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTACL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTADS3.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTAL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTCL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTCMD.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTeX.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexTxt2tags.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexVB.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexVerilog.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexVHDL.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexVisualProlog.cxx
  ${CMAKE_SOURCE_DIR}/external/scintilla/lexers/LexYAML.cxx
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

##############################################################################
# BONZOMATIC
file(GLOB BZC_PROJECT_SRCS
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/src/*.h
)
source_group("Bonzomatic" FILES ${BZC_PROJECT_SRCS})
if (APPLE)
  set(BZC_PLATFORM_SRCS
    ${CMAKE_SOURCE_DIR}/src/platform_glfw/Renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_common/FFT.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/MIDI.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Misc.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_x11/Timer.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/SetupDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/platform_osx/Clipboard.cpp
  )
  source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})
  set(BZC_RESOURCES_DATA
    ${CMAKE_SOURCE_DIR}/data/macosx/icon.icns
  )
  source_group("Data" FILES ${BZC_RESOURCES_DATA})
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
elseif (MSVC)
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

if (MSVC AND BONZOMATIC_NDI)
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
#### APPLE BUNDLE, RESSOURCES AND DYNAMIC LIBS
if (APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)

  # Define some settings for the Bundle
  set(MACOSX_BUNDLE_BUNDLE_NAME ${BZC_EXE_NAME})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "${BZC_EXE_NAME}")
  set(MACOSX_BUNDLE_ICON_FILE icon.icns)
  set(MACOSX_BUNDLE_INFO_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH},Copyright © 2016 The Bonzomatic Contributors")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2016 The Bonzomatic Contributors. All rights reserved.")

  set_source_files_properties(${BZC_RESOURCES_DATA} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  set(OSX_LIB_FILES ${CMAKE_SOURCE_DIR}/external/bass/libbass.dylib)
  set_source_files_properties(${OSX_LIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS)
  set(BZC_PROJECT_SRCS ${BZC_PROJECT_SRCS} ${OSX_LIB_FILES})

  set(BZC_PROJECT_SRCS ${GUI_TYPE} ${BZC_PROJECT_SRCS})

  find_library(COCOA_FRAMEWORK Cocoa)
  find_library(OPENGL_FRAMEWORK OpenGL)
  find_library(CARBON_FRAMEWORK Carbon)
  mark_as_advanced(COCOA_FRAMEWORK OPENGL_FRAMEWORK CARBON_FRAMEWORK)
  set(PLATFORM_LIBS ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${CARBON_FRAMEWORK})
elseif (UNIX)
  set(PLATFORM_LIBS GL)
elseif (MSVC)
    if (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX11")
        set(PLATFORM_LIBS d3d11 d3dcompiler dxguid DXGI winmm)
    elseif (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX9")
        set(PLATFORM_LIBS d3d9 d3dx9 winmm)
    else ()
        set(PLATFORM_LIBS opengl32 glu32 winmm)
    endif ()
endif ()
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${PLATFORM_LIBS})
##############################################################################

# create the executable
link_directories(${BZC_LINK_DIRS})
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
if (APPLE AND GLFW_USE_RETINA)
  # Add special plist to enable Retina display support
  set_target_properties(${BZC_EXE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/data/macosx/MacOSXBundleInfo.plist.in)
endif ()
# Set compiler specific flags
if (APPLE)
  #target_compile_options(${BZC_EXE_NAME} PUBLIC -Werror -pedantic-errors -Wall -Wno-format-security)
elseif (UNIX)
  if (NOT BONZOMATIC_64BIT)
  #  set_target_properties(${BZC_EXE_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
  endif ()
elseif (MSVC)
  if (BONZOMATIC_NDI)
    target_compile_definitions(${BZC_EXE_NAME} PUBLIC -DBONZOMATIC_ENABLE_NDI)
  endif ()
  set_target_properties(${BZC_EXE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
  target_compile_options(${BZC_EXE_NAME} PUBLIC "$<$<CONFIG:Release>:/MT>")
endif ()
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS})

if (MSVC)
  if (BONZOMATIC_64BIT)
    add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/bass/x64/bass.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
  else ()
    add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/bass/x86/bass.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
  endif ()
  if (BONZOMATIC_NDI)
    if (BONZOMATIC_64BIT)
      add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "c:/Program Files/NewTek/NewTek NDI SDK/Bin/x64/Processing.NDI.Lib.x64.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
    else ()
      add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "c:/Program Files/NewTek/NewTek NDI SDK/Bin/x86/Processing.NDI.Lib.x86.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
    endif ()
  endif ()
endif ()

## install
if (UNIX AND (NOT APPLE))
  if (BONZOMATIC_64BIT)
    install(FILES "${CMAKE_SOURCE_DIR}/external/bass/x64/libbass.so" DESTINATION lib)
  else ()
    install(FILES "${CMAKE_SOURCE_DIR}/external/bass/x86/libbass.so" DESTINATION lib)
  endif ()
  install(TARGETS ${BZC_EXE_NAME} RUNTIME DESTINATION bin)
endif ()
