set(CMAKE_OSX_ARCHITECTURES x86_64)
project(Bonzomatic)

cmake_minimum_required(VERSION 2.8)

set(BZC_EXE_NAME "Bonzomatic")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")

option(BONZOMATIC_64BIT "Compile for 64 bit target?" ON)
set(BONZOMATIC_WINDOWS_FLAVOR "DX11" CACHE STRING "Windows renderer flavor selected at CMake configure time (DX11, DX9 or SDL)")
set_property(CACHE BONZOMATIC_WINDOWS_FLAVOR PROPERTY STRINGS DX11 DX9 SDL) 

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

add_definitions(-DSCI_LEXER -DSCI_NAMESPACE)

# Dont compile sdl and glew for windows dx targets
if ( APPLE OR UNIX OR (MSVC AND (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "SDL")))
##############################################################################
# SDL
if (APPLE OR UNIX)
	find_package(SDL2 REQUIRED)
	if (NOT SDL2_FOUND)
		message(FATAL_ERROR "SDL2 not found!")
	endif ()
	# fix sdl whitespaces problems
	string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
	set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${SDL2_INCLUDE_DIRS})
	if (APPLE)
		find_library(COCOA_FRAMEWORK Cocoa)
		find_library(OPENGL_FRAMEWORK OpenGL)
		find_library(CARBON_FRAMEWORK Carbon)
		set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${CARBON_FRAMEWORK} ${SDL2_LIBRARIES} SDL2main)
	elseif (UNIX)
		set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${SDL2_LIBRARIES} SDL2main)
	endif ()
elseif (MSVC)
	set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/sdl/include)
	set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/sdl/lib/SDL2.lib ${CMAKE_SOURCE_DIR}/external/sdl/lib/SDL2main.lib)
endif ()

##############################################################################
# GLEW
set(GLEW_SRCS
${CMAKE_SOURCE_DIR}/external/glew/glew.c
)
add_library(bzc_glew STATIC ${GLEW_SRCS})
target_include_directories(bzc_glew PUBLIC ${CMAKE_SOURCE_DIR}/external/glew)
target_compile_definitions(bzc_glew PUBLIC -DGLEW_STATIC)
set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glew)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_glew)
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
		set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/libbass64.so)
	else ()
		set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/libbass.so)
	endif ()
elseif (MSVC)
	set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/bass/bass.lib)
endif ()

##############################################################################
# JSONXX
set(JSONXX_SRCS
	${CMAKE_SOURCE_DIR}/external/jsonxx/jsonxx.cc
)
add_library(bzc_jsonxx STATIC ${JSONXX_SRCS})
target_include_directories(bzc_jsonxx PUBLIC ${CMAKE_SOURCE_DIR}/external/jsonxx)
set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/jsonxx)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_jsonxx)

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
		${CMAKE_SOURCE_DIR}/src/platform_sdl/Renderer.cpp
		${CMAKE_SOURCE_DIR}/src/platform_sdl/FFT.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/MIDI.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Misc.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/SetupDialog.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Timer.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Platform_DynamicLoad.cpp
		${CMAKE_SOURCE_DIR}/src/platform_osx/Clipboard.cpp
	)
	source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})
	set(BZC_RESOURCES_DATA
		${CMAKE_SOURCE_DIR}/data/macosx/icon.icns
	)
	source_group("Data" FILES ${BZC_RESOURCES_DATA})
elseif (UNIX)
	set(BZC_PLATFORM_SRCS
		${CMAKE_SOURCE_DIR}/src/platform_sdl/Renderer.cpp
		${CMAKE_SOURCE_DIR}/src/platform_sdl/FFT.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/MIDI.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Misc.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/SetupDialog.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Timer.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Platform_DynamicLoad.cpp
		${CMAKE_SOURCE_DIR}/src/platform_x11/Clipboard.cpp
	)
	source_group("Bonzomatic\\Platform" FILES ${BZC_PLATFORM_SRCS})
elseif (MSVC)
	if (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX11")
		set(BZC_PLATFORM_SRCS
			${CMAKE_SOURCE_DIR}/src/platform_w32_dx11/Renderer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_sdl/FFT.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Platform_DynamicLoad.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Clipboard.cpp
			${CMAKE_SOURCE_DIR}/data/windows/resource.h
		)
	elseif (${BONZOMATIC_WINDOWS_FLAVOR} MATCHES "DX9")
		set(BZC_PLATFORM_SRCS
			${CMAKE_SOURCE_DIR}/src/platform_w32_dx9/Renderer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_sdl/FFT.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Platform_DynamicLoad.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Clipboard.cpp
			${CMAKE_SOURCE_DIR}/data/windows/resource.h
		)
	else ()
		set(BZC_PLATFORM_SRCS
			${CMAKE_SOURCE_DIR}/src/platform_sdl/Renderer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_sdl/FFT.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/MIDI.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Misc.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/SetupDialog.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Timer.cpp
			${CMAKE_SOURCE_DIR}/src/platform_w32_common/Platform_DynamicLoad.cpp
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

set(BZC_PROJECT_SRCS ${BZC_PROJECT_SRCS} ${BZC_PLATFORM_SRCS} ${BZC_RESOURCES_DATA})

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
add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
if (APPLE AND GLFW_USE_RETINA)
	# Add special plist to enable Retina display support
	set_target_properties(${BZC_EXE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/data/macosx/MacOSXBundleInfo.plist.in)
endif ()
# Set compiler specific flags
if (APPLE)
	#target_compile_options(${BZC_EXE_NAME} PUBLIC -Werror -pedantic-errors -Wall -Wno-format-security)
elseif (UNIX)
	#target_compile_options(${BZC_EXE_NAME} PUBLIC -Werror -pedantic-errors -Wall -Wno-format-security)
elseif (MSVC)
	set_target_properties(${BZC_EXE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	target_compile_options(${BZC_EXE_NAME} PUBLIC /W4)
endif ()
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS})

if (MSVC)
	add_custom_command(TARGET ${BZC_EXE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/bass/bass.dll" $<TARGET_FILE_DIR:${BZC_EXE_NAME}>)
endif ()
