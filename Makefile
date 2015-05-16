.SUFFIXES:
all: Bonzomatic

SOURCES_C := \
	external/glee/GLee.c \
	external/stb_image.c

SOURCES_CC := \
	external/jsonxx/jsonxx.cc

SOURCES_CXX := \
	external/scintilla/lexers/LexA68k.cxx \
	external/scintilla/lexers/LexAbaqus.cxx \
	external/scintilla/lexers/LexAda.cxx \
	external/scintilla/lexers/LexAPDL.cxx \
	external/scintilla/lexers/LexAsm.cxx \
	external/scintilla/lexers/LexAsn1.cxx \
	external/scintilla/lexers/LexASY.cxx \
	external/scintilla/lexers/LexAU3.cxx \
	external/scintilla/lexers/LexAVE.cxx \
	external/scintilla/lexers/LexAVS.cxx \
	external/scintilla/lexers/LexBaan.cxx \
	external/scintilla/lexers/LexBash.cxx \
	external/scintilla/lexers/LexBasic.cxx \
	external/scintilla/lexers/LexBibTeX.cxx \
	external/scintilla/lexers/LexBullant.cxx \
	external/scintilla/lexers/LexCaml.cxx \
	external/scintilla/lexers/LexCLW.cxx \
	external/scintilla/lexers/LexCmake.cxx \
	external/scintilla/lexers/LexCOBOL.cxx \
	external/scintilla/lexers/LexCoffeeScript.cxx \
	external/scintilla/lexers/LexConf.cxx \
	external/scintilla/lexers/LexCPP.cxx \
	external/scintilla/lexers/LexCrontab.cxx \
	external/scintilla/lexers/LexCsound.cxx \
	external/scintilla/lexers/LexCSS.cxx \
	external/scintilla/lexers/LexD.cxx \
	external/scintilla/lexers/LexDMAP.cxx \
	external/scintilla/lexers/LexDMIS.cxx \
	external/scintilla/lexers/LexECL.cxx \
	external/scintilla/lexers/LexEiffel.cxx \
	external/scintilla/lexers/LexErlang.cxx \
	external/scintilla/lexers/LexEScript.cxx \
	external/scintilla/lexers/LexFlagship.cxx \
	external/scintilla/lexers/LexForth.cxx \
	external/scintilla/lexers/LexFortran.cxx \
	external/scintilla/lexers/LexGAP.cxx \
	external/scintilla/lexers/LexGui4Cli.cxx \
	external/scintilla/lexers/LexHaskell.cxx \
	external/scintilla/lexers/LexHTML.cxx \
	external/scintilla/lexers/LexInno.cxx \
	external/scintilla/lexers/LexKix.cxx \
	external/scintilla/lexers/LexKVIrc.cxx \
	external/scintilla/lexers/LexLaTeX.cxx \
	external/scintilla/lexers/LexLisp.cxx \
	external/scintilla/lexers/LexLout.cxx \
	external/scintilla/lexers/LexLua.cxx \
	external/scintilla/lexers/LexMagik.cxx \
	external/scintilla/lexers/LexMarkdown.cxx \
	external/scintilla/lexers/LexMatlab.cxx \
	external/scintilla/lexers/LexMetapost.cxx \
	external/scintilla/lexers/LexMMIXAL.cxx \
	external/scintilla/lexers/LexModula.cxx \
	external/scintilla/lexers/LexMPT.cxx \
	external/scintilla/lexers/LexMSSQL.cxx \
	external/scintilla/lexers/LexMySQL.cxx \
	external/scintilla/lexers/LexNimrod.cxx \
	external/scintilla/lexers/LexNsis.cxx \
	external/scintilla/lexers/LexOpal.cxx \
	external/scintilla/lexers/LexOScript.cxx \
	external/scintilla/lexers/LexOthers.cxx \
	external/scintilla/lexers/LexPascal.cxx \
	external/scintilla/lexers/LexPB.cxx \
	external/scintilla/lexers/LexPerl.cxx \
	external/scintilla/lexers/LexPLM.cxx \
	external/scintilla/lexers/LexPO.cxx \
	external/scintilla/lexers/LexPOV.cxx \
	external/scintilla/lexers/LexPowerPro.cxx \
	external/scintilla/lexers/LexPowerShell.cxx \
	external/scintilla/lexers/LexProgress.cxx \
	external/scintilla/lexers/LexPS.cxx \
	external/scintilla/lexers/LexPython.cxx \
	external/scintilla/lexers/LexR.cxx \
	external/scintilla/lexers/LexRebol.cxx \
	external/scintilla/lexers/LexRegistry.cxx \
	external/scintilla/lexers/LexRuby.cxx \
	external/scintilla/lexers/LexRust.cxx \
	external/scintilla/lexers/LexScriptol.cxx \
	external/scintilla/lexers/LexSmalltalk.cxx \
	external/scintilla/lexers/LexSML.cxx \
	external/scintilla/lexers/LexSorcus.cxx \
	external/scintilla/lexers/LexSpecman.cxx \
	external/scintilla/lexers/LexSpice.cxx \
	external/scintilla/lexers/LexSQL.cxx \
	external/scintilla/lexers/LexSTTXT.cxx \
	external/scintilla/lexers/LexTACL.cxx \
	external/scintilla/lexers/LexTADS3.cxx \
	external/scintilla/lexers/LexTAL.cxx \
	external/scintilla/lexers/LexTCL.cxx \
	external/scintilla/lexers/LexTCMD.cxx \
	external/scintilla/lexers/LexTeX.cxx \
	external/scintilla/lexers/LexTxt2tags.cxx \
	external/scintilla/lexers/LexVB.cxx \
	external/scintilla/lexers/LexVerilog.cxx \
	external/scintilla/lexers/LexVHDL.cxx \
	external/scintilla/lexers/LexVisualProlog.cxx \
	external/scintilla/lexers/LexYAML.cxx \
	external/scintilla/lexlib/Accessor.cxx \
	external/scintilla/lexlib/CharacterCategory.cxx \
	external/scintilla/lexlib/CharacterSet.cxx \
	external/scintilla/lexlib/LexerBase.cxx \
	external/scintilla/lexlib/LexerModule.cxx \
	external/scintilla/lexlib/LexerNoExceptions.cxx \
	external/scintilla/lexlib/LexerSimple.cxx \
	external/scintilla/lexlib/PropSetSimple.cxx \
	external/scintilla/lexlib/StyleContext.cxx \
	external/scintilla/lexlib/WordList.cxx \
	external/scintilla/src/AutoComplete.cxx \
	external/scintilla/src/CallTip.cxx \
	external/scintilla/src/CaseConvert.cxx \
	external/scintilla/src/CaseFolder.cxx \
	external/scintilla/src/Catalogue.cxx \
	external/scintilla/src/CellBuffer.cxx \
	external/scintilla/src/CharClassify.cxx \
	external/scintilla/src/ContractionState.cxx \
	external/scintilla/src/Decoration.cxx \
	external/scintilla/src/Document.cxx \
	external/scintilla/src/EditModel.cxx \
	external/scintilla/src/Editor.cxx \
	external/scintilla/src/EditView.cxx \
	external/scintilla/src/ExternalLexer.cxx \
	external/scintilla/src/Indicator.cxx \
	external/scintilla/src/KeyMap.cxx \
	external/scintilla/src/LineMarker.cxx \
	external/scintilla/src/MarginView.cxx \
	external/scintilla/src/PerLine.cxx \
	external/scintilla/src/PositionCache.cxx \
	external/scintilla/src/RESearch.cxx \
	external/scintilla/src/RunStyles.cxx \
	external/scintilla/src/ScintillaBase.cxx \
	external/scintilla/src/Selection.cxx \
	external/scintilla/src/Style.cxx \
	external/scintilla/src/UniConversion.cxx \
	external/scintilla/src/ViewStyle.cxx \
	external/scintilla/src/XPM.cxx

SOURCES_CPP := \
	main.cpp \
	Platform.cpp \
	ShaderEditor.cpp \
	platform_sdl/Renderer.cpp \
	platform_x11/Clipboard.cpp \
	platform_x11/FFT.cpp \
	platform_x11/SetupDialog.cpp \
	platform_x11/Timer.cpp \
	platform_x11/Platform_DynamicLoad.cpp

INCLUDEPATHS := \
  external \
  external/scintilla/include \
	external/scintilla/lexlib \
	external/scintilla/src \
	external/glee \
	external/sdl/include \
	external/bass

CXX ?= cpp
OBJDIR ?= .obj

CXXFLAGS := -std=c++11 -arch x86_64 -Os -Wall -DSCI_LEXER -DSCI_NAMESPACE -DGTK `pkg-config --cflags sdl`
CXXFLAGS += $(foreach p,$(INCLUDEPATHS),$(addprefix -I,$p))
#CXXFLAGS += -Werror
LDFLAGS := -framework OpenGL -framework Cocoa `pkg-config --libs sdl`

define MAKE_RULES
  $1.MODULE := $(addprefix $(OBJDIR)/, $(1:$(2)=$(3)))
  $1.DEPFILE := $(addprefix $(OBJDIR)/, $(1:$(2)=$(4)))
  MODULES += $$($1.MODULE)
  DEPFILES += $$($1.DEPFILE)

$$($1.MODULE): $1 $$($1.DEPFILE) Makefile
	$(CXX) $(CXXFLAGS) -c -o $$($1.MODULE) $1

$$($1.DEPFILE): $1 Makefile
	@mkdir -p $$(dir $$($1.DEPFILE))
	$(CXX) $(CXXFLAGS) -MM $1 -MT $$($1.MODULE) -MT $$($1.DEPFILE) -MF $$($1.DEPFILE)
endef

# Generate rules for .d and .o files
$(foreach src,$(SOURCES_C), $(eval $(call MAKE_RULES,$(src),.c,.o,.d)))
$(foreach src,$(SOURCES_CC), $(eval $(call MAKE_RULES,$(src),.cc,.oc,.dc)))
$(foreach src,$(SOURCES_CXX), $(eval $(call MAKE_RULES,$(src),.cxx,.oxx,.dxx)))
$(foreach src,$(SOURCES_CPP), $(eval $(call MAKE_RULES,$(src),.cpp,.opp,.dpp)))

-include $(DEPFILES)

Bonzomatic: $(MODULES) Makefile
	$(CXX) $(MODULES) $(LDFLAGS) -o $@

clean:
	@rm -r Bonzomatic $(OBJDIR)

