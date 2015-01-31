#include <stdexcept>
#include <new>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <Scintilla.h>
#include <Platform.h>

#include <ILexer.h>

#ifdef SCI_LEXER
#include <SciLexer.h>
#endif
#include <StringCopy.h>
#ifdef SCI_LEXER
#include <LexerModule.h>
#endif
#include <SplitVector.h>
#include <Partitioning.h>
#include <RunStyles.h>
#include <ContractionState.h>
#include <CellBuffer.h>
#include <CallTip.h>
#include <KeyMap.h>
#include <Indicator.h>
#include <XPM.h>
#include <LineMarker.h>
#include <Style.h>
#include <ViewStyle.h>
#include <CharClassify.h>
#include <Decoration.h>
#include <CaseFolder.h>
#include <Document.h>
#include <CaseConvert.h>
#include <UniConversion.h>
#include <Selection.h>
#include <PositionCache.h>
#include <EditModel.h>
#include <MarginView.h>
#include <EditView.h>
#include <Editor.h>
#include <AutoComplete.h>
#include <ScintillaBase.h>

#ifdef SCI_LEXER
#include <ExternalLexer.h>
#endif

struct SHADEREDITOR_OPTIONS {
  std::string sFontPath;
  int nFontSize;
  Scintilla::PRectangle rect;
};

class ShaderEditor : public Scintilla::Editor
{
  Scintilla::Surface *surfaceWindow;
  Scintilla::LexState * lexState;
  bool bReadOnly;
  std::string sFontFile;
  int nFontSize;
  bool bHasMouseCapture;
public:
  ShaderEditor(Scintilla::Surface *surfaceWindow);

  void Initialise();
  void Initialise(SHADEREDITOR_OPTIONS &options);
  void SetVerticalScrollPos();
  void SetHorizontalScrollPos();
  bool ModifyScrollBars(int nMax, int nPage);
  void Copy();
  void Paste();
  void ClaimSelection();
  void NotifyChange();
  void NotifyParent(Scintilla::SCNotification scn);
  void CopyToClipboard(const Scintilla::SelectionText &selectedText);
  void SetMouseCapture(bool on);
  bool HaveMouseCapture();
  sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
  void SetTicking(bool on);

  void SetText( char * buf );
  void GetText( char * buf, int len );

  void Paint();
  void SetAStyle(int style, Scintilla::ColourDesired fore, Scintilla::ColourDesired back=0xFFFFFFFF, int size=-1, const char *face=0);
  void Tick();
  int KeyDown(int key, bool shift, bool ctrl, bool alt, bool *consumed);
  void ButtonDown( Scintilla::Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt );
  void ButtonMovePublic( Scintilla::Point pt );
  void ButtonUp( Scintilla::Point pt, unsigned int curTime, bool ctrl );
  void AddCharUTF( const char *s, unsigned int len, bool treatAsDBCS=false );
  void NotifyStyleToNeeded(int endStyleNeeded);

  void SetReadOnly( bool );
};
