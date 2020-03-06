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

enum AutoIndentationType {
  aitNone,
  aitPreserve,
  aitSmart
};

struct SHADEREDITOR_THEME {
  unsigned int text             = 0xFFFFFFFF;
  unsigned int comment          = 0xFF00FF00;
  unsigned int number           = 0xFF0080FF;
  unsigned int op               = 0xFF00CCFF;
  unsigned int keyword          = 0xFF0066FF;
  unsigned int type             = 0xFFFFFF00;
  unsigned int builtin          = 0xFF88FF44;
  unsigned int preprocessor     = 0xFFC0C0C0;
  unsigned int selection        = 0xC0CC9966;
  unsigned int charBackground   = 0xC0000000;
  bool bUseCharBackground = false;
};

struct SHADEREDITOR_OPTIONS {
  std::string sFontPath;
  int nFontSize;
  Scintilla::PRectangle rect;
  unsigned char nOpacity;
  bool bUseSpacesForTabs;
  int nTabSize;
  bool bVisibleWhitespace;
  AutoIndentationType eAutoIndent;
  SHADEREDITOR_THEME theme;
};

class ShaderEditor : public Scintilla::Editor
{
  Scintilla::Surface *surfaceWindow;
  Scintilla::LexState * lexState;
  bool bReadOnly;
  bool bHasMouseCapture;

  std::string sFontFile;
  int nFontSize;
  unsigned char nOpacity;
  bool bUseSpacesForTabs;
  int nTabSize;
  bool bVisibleWhitespace;
  AutoIndentationType eAutoIndent;
  SHADEREDITOR_THEME theme;

public:
  ShaderEditor(Scintilla::Surface *surfaceWindow);

  void Initialise();
  void Initialise(SHADEREDITOR_OPTIONS &options);

  void SetPosition( Scintilla::PRectangle rect );

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
  bool FineTickerRunning(TickReason);
  void FineTickerStart(TickReason, int, int);
  void FineTickerCancel(TickReason);

  void SetText( const char * buf );
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
  Scintilla::Font * GetTextFont();
    
private:
  enum IndentationStatus {
    isNone,        // no effect on indentation
    isBlockStart,  // indentation block begin such as "{" or VB "function"
    isBlockEnd,    // indentation end indicator such as "}" or VB "end"
    isKeyWordStart // Keywords that cause indentation
  };
  
  int GetLineLength(int line);
  int GetCurrentLineNumber();
  Scintilla::Sci_CharacterRange GetSelection();
  int GetLineIndentation(int line);
  int GetLineIndentPosition(int line);
  void SetLineIndentation(int line, int indent);
  void PreserveIndentation(char ch);
  std::vector<std::string> GetLinePartsInStyle(int line, int style);
  bool isAStatementIndent(std::string &word);
  IndentationStatus GetIndentState(int line);
  int IndentOfBlock(int line);
  bool RangeIsAllWhitespace(int start, int end);
  void AutomaticIndentation(char ch);
};
