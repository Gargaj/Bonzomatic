#include <cstring>
#include "ShaderEditor.h"
#include "Renderer.h"
#include "PropSetSimple.h"
#include "Clipboard.h"

const int statementLookback = 20;
const int statementIndentStyleNumber = 5;
const int blockAndStatementEndStyleNumber = 10;
std::string statementIndent[] = { "case", "default", "do", "else", "for", "if", "while" };
const char statementEnd = ';';
const char blockStart = '{';
const char blockEnd = '}';

ShaderEditor::ShaderEditor( Scintilla::Surface *s )
{
  bReadOnly = false;
  surfaceWindow = s;
  sFontFile = "";
  nFontSize = 16;
  bHasMouseCapture = false;
  nOpacity = 0xC0;
}

void ShaderEditor::SetAStyle(int style, Scintilla::ColourDesired fore, Scintilla::ColourDesired back, int size, const char *face)
{
  WndProc(SCI_STYLESETFORE, style, (sptr_t)fore.AsLong());
  WndProc(SCI_STYLESETBACK, style, (sptr_t)back.AsLong());
  if (size >= 1)
    WndProc(SCI_STYLESETSIZE, style, size);
  if (face) 
    WndProc(SCI_STYLESETFONT, style, reinterpret_cast<sptr_t>(face));
}

#define BACKGROUND(x) ( (x) | (nOpacity << 24) )

const size_t NB_FOLDER_STATE = 7;
const size_t FOLDER_TYPE = 0;

const int markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};
extern const char * shaderKeyword;
extern const char * shaderType;
extern const char * shaderBuiltin;

using namespace Scintilla;
class Scintilla::LexState : public LexInterface {
  const LexerModule *lexCurrent;
  void SetLexerModule(const LexerModule *lex);
  PropSetSimple props;
  int interfaceVersion;
public:
  int lexLanguage;

  explicit LexState(Document *pdoc_);
  virtual ~LexState();
  void SetLexer(uptr_t wParam);
  void SetLexerLanguage(const char *languageName);
  const char *DescribeWordListSets();
  void SetWordList(int n, const char *wl);
  const char *GetName() const;
  void *PrivateCall(int operation, void *pointer);
  const char *PropertyNames();
  int PropertyType(const char *name);
  const char *DescribeProperty(const char *name);
  void PropSet(const char *key, const char *val);
  const char *PropGet(const char *key) const;
  int PropGetInt(const char *key, int defaultValue=0) const;
  int PropGetExpanded(const char *key, char *result) const;

  int LineEndTypesSupported();
  int AllocateSubStyles(int styleBase, int numberStyles);
  int SubStylesStart(int styleBase);
  int SubStylesLength(int styleBase);
  int StyleFromSubStyle(int subStyle);
  int PrimaryStyleFromStyle(int style);
  void FreeSubStyles();
  void SetIdentifiers(int style, const char *identifiers);
  int DistanceToSecondaryStyles();
  const char *GetSubStyleBases();
};

static unsigned int wndID = 1;
void ShaderEditor::Initialise()
{
  wMain = (Scintilla::WindowID)(unsigned long)(wndID++);

  lexState = new Scintilla::LexState( pdoc );

  WndProc( SCI_SETBUFFEREDDRAW, 0, 0 );
  WndProc( SCI_SETCODEPAGE, SC_CP_UTF8, 0 );

  WndProc( SCI_SETWRAPMODE, SC_WRAP_WORD, 0 );

  //WndProc( SCI_SETLEXERLANGUAGE, SCLEX_CPP, 0 );

  SetAStyle( STYLE_DEFAULT,     0xFFFFFFFF, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );
  WndProc( SCI_STYLECLEARALL, 0, 0 );
  SetAStyle( STYLE_LINENUMBER,  0xFFC0C0C0, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );
  SetAStyle( STYLE_BRACELIGHT,  0xFF00FF00, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );
  SetAStyle( STYLE_BRACEBAD,    0xFF0000FF, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );
  SetAStyle( STYLE_INDENTGUIDE, 0xFFC0C0C0, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );

  WndProc(SCI_SETFOLDMARGINCOLOUR,   1, BACKGROUND( 0x1A1A1A ));
  WndProc(SCI_SETFOLDMARGINHICOLOUR, 1, BACKGROUND( 0x1A1A1A ));
  WndProc(SCI_SETSELBACK,            1, BACKGROUND( 0xCC9966 ));

  SetReadOnly(false);

  for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
  {
    WndProc(SCI_MARKERDEFINE,  markersArray[FOLDER_TYPE][i], markersArray[4][i]);
    WndProc(SCI_MARKERSETBACK, markersArray[FOLDER_TYPE][i], 0xFF6A6A6A);
    WndProc(SCI_MARKERSETFORE, markersArray[FOLDER_TYPE][i], 0xFF333333);
  }
  WndProc(SCI_SETUSETABS, bUseSpacesForTabs ? 0 : 1, 0);
  WndProc(SCI_SETTABWIDTH, nTabSize, 0);
  WndProc(SCI_SETINDENT, nTabSize, 0);
  WndProc(SCI_SETINDENTATIONGUIDES, SC_IV_REAL, 0);

  if (bVisibleWhitespace)
  {
    WndProc(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS, 0);
    WndProc(SCI_SETWHITESPACEFORE, 1, 0x30FFFFFF);
    WndProc(SCI_SETWHITESPACESIZE, 2, 0 );
  }
  
  lexState->SetLexer( SCLEX_CPP );
  lexState->SetWordList(0, shaderKeyword);
  lexState->SetWordList(1, shaderType);
  lexState->SetWordList(3, shaderBuiltin);

  SetAStyle(SCE_C_DEFAULT,      0xFFFFFFFF, BACKGROUND( 0x000000 ), nFontSize, sFontFile.c_str() );
  SetAStyle(SCE_C_WORD,         0xFF0066FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_WORD2,        0xFFFFFF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_GLOBALCLASS,  0xFF88FF44, BACKGROUND( 0x000000 ));  
  SetAStyle(SCE_C_PREPROCESSOR, 0xFFC0C0C0, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_NUMBER,       0xFF0080FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_OPERATOR,     0xFF00CCFF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENT,      0xFF00FF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENTLINE,  0xFF00FF00, BACKGROUND( 0x000000 ));
  
  lexState->Colourise( 0, -1 );

  //WndProc( SCI_COLOURISE, 0, 0 );

  vs.Refresh( *surfaceWindow, 4 );
}

void ShaderEditor::Initialise( SHADEREDITOR_OPTIONS &options )
{
  nFontSize = options.nFontSize;
  sFontFile = options.sFontPath;
  nOpacity = options.nOpacity;
  bUseSpacesForTabs = options.bUseSpacesForTabs;
  nTabSize = options.nTabSize;
  bVisibleWhitespace = options.bVisibleWhitespace;
  eAutoIndent = options.eAutoIndent;

  Initialise();
  SetPosition( options.rect );
}

void ShaderEditor::SetVerticalScrollPos()
{

}

void ShaderEditor::SetHorizontalScrollPos()
{

}

bool ShaderEditor::ModifyScrollBars( int nMax, int nPage )
{
  return true;
}

void ShaderEditor::Copy()
{
  if (!sel.Empty()) {
    SelectionText selectedText;
    CopySelectionRange(&selectedText);
    CopyToClipboard(selectedText);
  }
}

void ShaderEditor::Paste()
{
  int n = Clipboard::GetContentsLength();
  if (n == 0) return;

  char * p = new char[n + 1];
  memset(p,0,n+1);
  Clipboard::GetContents( p, n );

  ClearSelection();
  InsertPaste(p, n);

  delete[] p;
}

void ShaderEditor::ClaimSelection()
{

}

void ShaderEditor::NotifyChange()
{

}

void ShaderEditor::NotifyParent( Scintilla::SCNotification scn )
{
  switch (scn.nmhdr.code) {
    case SCN_CHARADDED:
      char ch = static_cast<char>(scn.ch);
      if(eAutoIndent == aitPreserve) {
        PreserveIndentation(ch);
      } else if (aitSmart) {
        AutomaticIndentation(ch);
      }
      break;
    }
}

void ShaderEditor::CopyToClipboard( const Scintilla::SelectionText &selectedText )
{
  Clipboard::Copy( selectedText.Data(), (int)selectedText.Length() );
}

void ShaderEditor::SetMouseCapture( bool on )
{
  bHasMouseCapture = on;
}

bool ShaderEditor::HaveMouseCapture()
{
  return bHasMouseCapture;
}

sptr_t ShaderEditor::DefWndProc( unsigned int iMessage, uptr_t wParam, sptr_t lParam )
{
  return 0;
}

void ShaderEditor::Paint()
{
  Renderer::SetTextRenderingViewport( wMain.GetPosition() );
  Scintilla::Editor::Paint( surfaceWindow, GetClientRectangle() );
}

void ShaderEditor::SetText( const char * buf )
{
  WndProc( SCI_SETREADONLY, false, 0 );
  WndProc( SCI_CLEARALL, false, 0 );
  WndProc( SCI_SETUNDOCOLLECTION, 0, 0);
  WndProc( SCI_ADDTEXT, strlen(buf), (sptr_t)buf );
  WndProc( SCI_SETUNDOCOLLECTION, 1, 0);
  WndProc( SCI_SETREADONLY, bReadOnly, 0 );
  WndProc( SCI_GOTOPOS, 0, 0 );
  if (!bReadOnly)
    SetFocusState( true );
}

void ShaderEditor::Tick()
{
  Scintilla::Editor::Tick();
}

void ShaderEditor::SetTicking( bool on )
{

}

int ShaderEditor::KeyDown( int key, bool shift, bool ctrl, bool alt, bool *consumed )
{
  return Scintilla::Editor::KeyDown( key, shift, ctrl, alt, consumed );
}

void ShaderEditor::AddCharUTF( const char *s, unsigned int len, bool treatAsDBCS )
{
  Scintilla::Editor::AddCharUTF( s, len, treatAsDBCS );
}

void ShaderEditor::GetText( char * buf, int len )
{
  memset( buf, 0, len );

  int lengthDoc = (int)WndProc( SCI_GETLENGTH, 0, 0 );

  Scintilla::TextRange tr;
  tr.chrg.cpMin = 0;
  tr.chrg.cpMax = Scintilla::Platform::Minimum( len - 1, lengthDoc );
  tr.lpstrText  = buf;

  WndProc(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
}

void ShaderEditor::NotifyStyleToNeeded(int endStyleNeeded) {
#ifdef SCI_LEXER
  if (lexState->lexLanguage != SCLEX_CONTAINER) {
    int lineEndStyled = pdoc->LineFromPosition(pdoc->GetEndStyled());
    int endStyled = pdoc->LineStart(lineEndStyled);
    lexState->Colourise(endStyled, endStyleNeeded);
    return;
  }
#endif
  Scintilla::Editor::NotifyStyleToNeeded(endStyleNeeded);
}

void ShaderEditor::SetReadOnly( bool b )
{
  bReadOnly = b;
  WndProc( SCI_SETREADONLY, bReadOnly, 0 );
  if (bReadOnly)
  {
    WndProc(SCI_SETVIEWWS, SCWS_INVISIBLE, 0);
    WndProc(SCI_SETMARGINWIDTHN, 0, 0);
    WndProc(SCI_SETMARGINWIDTHN, 1, 0);
    WndProc( SCI_SETCARETLINEVISIBLE,   0, 0);
    WndProc( SCI_SETCARETFORE,          0, 0);
  }
  else
  {
    WndProc(SCI_SETMARGINWIDTHN, 0, 44);//Calculate correct width
    WndProc(SCI_SETMARGINWIDTHN, 1, 20);//Calculate correct width
    WndProc(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);//Calculate correct width

    WndProc( SCI_SETCARETFORE,          0xFFFFFFFF, 0);
    WndProc( SCI_SETCARETLINEVISIBLE,   1, 0);
    WndProc( SCI_SETCARETLINEBACK,      0xFFFFFFFF, 0);
    WndProc( SCI_SETCARETLINEBACKALPHA, 0x20, 0);
  }
}

void ShaderEditor::ButtonDown( Scintilla::Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt )
{
  Scintilla::PRectangle rect = wMain.GetPosition();
  pt.x -= rect.left;
  pt.y -= rect.top;
  Scintilla::Editor::ButtonDown( pt, curTime, shift, ctrl, alt );
}

void ShaderEditor::ButtonMovePublic( Scintilla::Point pt )
{
  Scintilla::PRectangle rect = wMain.GetPosition();
  pt.x -= rect.left;
  pt.y -= rect.top;
  ButtonMove(pt);
}

void ShaderEditor::ButtonUp( Scintilla::Point pt, unsigned int curTime, bool ctrl )
{
  Scintilla::PRectangle rect = wMain.GetPosition();
  pt.x -= rect.left;
  pt.y -= rect.top;
  Scintilla::Editor::ButtonUp( pt, curTime, ctrl );
}

Font * ShaderEditor::GetTextFont()
{
  return &vs.styles[ STYLE_DEFAULT ].font;
}

void ShaderEditor::SetPosition( Scintilla::PRectangle rect )
{
  wMain.SetPosition(rect);
}

bool ShaderEditor::FineTickerRunning( TickReason )
{
  return false;
}

void ShaderEditor::FineTickerStart( TickReason, int, int )
{

}

void ShaderEditor::FineTickerCancel( TickReason )
{

}


/////////////////////////////////////////////////////////////////////////////////////////////
// Indentation handling

int ShaderEditor::GetLineLength(int line) {
  return WndProc(SCI_GETLINEENDPOSITION, line, 0) - WndProc(SCI_POSITIONFROMLINE, line, 0);
}

int ShaderEditor::GetCurrentLineNumber() {
  return WndProc(SCI_LINEFROMPOSITION, WndProc(SCI_GETCURRENTPOS, 0, 0), 0);
}

Sci_CharacterRange ShaderEditor::GetSelection() {
  Sci_CharacterRange chrange;
  chrange.cpMin = WndProc(SCI_GETSELECTIONSTART, 0, 0);
  chrange.cpMax = WndProc(SCI_GETSELECTIONEND, 0, 0);
  return chrange;
}

int ShaderEditor::GetLineIndentation(int line) {
  return WndProc(SCI_GETLINEINDENTATION, line, 0);
}

int ShaderEditor::GetLineIndentPosition(int line) {
  return WndProc(SCI_GETLINEINDENTPOSITION, line, 0);
}

void ShaderEditor::SetLineIndentation(int line, int indent) {
  if (indent < 0)
    return;
  Sci_CharacterRange crange = GetSelection();
  int posBefore = GetLineIndentPosition(line);
  WndProc(SCI_SETLINEINDENTATION, line, indent);
  int posAfter = GetLineIndentPosition(line);
  int posDifference = posAfter - posBefore;
  if (posAfter > posBefore) {
    // Move selection on
    if (crange.cpMin >= posBefore) {
      crange.cpMin += posDifference;
    }
    if (crange.cpMax >= posBefore) {
      crange.cpMax += posDifference;
    }
  } else if (posAfter < posBefore) {
    // Move selection back
    if (crange.cpMin >= posAfter) {
      if (crange.cpMin >= posBefore) {
        crange.cpMin += posDifference;
      } else {
        crange.cpMin = posAfter;
      }
    }
    if (crange.cpMax >= posAfter) {
      if (crange.cpMax >= posBefore) {
        crange.cpMax += posDifference;
      } else {
        crange.cpMax = posAfter;
      }
    }
  }
  SetSelection(static_cast<int>(crange.cpMin), static_cast<int>(crange.cpMax));
}

void ShaderEditor::PreserveIndentation(char ch) {
  int eolMode = WndProc(SCI_GETEOLMODE, 0, 0);
  int curLine = GetCurrentLineNumber();
  int lastLine = curLine - 1;

  if (((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') || (eolMode == SC_EOL_CR && ch == '\r')) {
    while (lastLine >= 0 && GetLineLength(lastLine) == 0) {
      lastLine--;
    }
    int indentAmount = 0;
    if (lastLine >= 0) {
      indentAmount = GetLineIndentation(lastLine);
    }
    if (indentAmount > 0) {
      SetLineIndentation(curLine, indentAmount);
    }
  }
}

std::vector<std::string> ShaderEditor::GetLinePartsInStyle(int line, int style) {
  std::vector<std::string> out;
  std::string s;
  int thisLineStart = WndProc(SCI_POSITIONFROMLINE, line, 0);
  int nextLineStart = WndProc(SCI_POSITIONFROMLINE, line + 1, 0);
  for (int pos = thisLineStart; pos < nextLineStart; pos++) {
    if (static_cast<char>(WndProc(SCI_GETSTYLEAT, pos, 0)) == style) {
      char c = WndProc(SCI_GETCHARAT, pos, 0);
      // special handling for statements and blocks characters as they should be dissociated to other strings
      if (c == statementEnd || c == blockStart || c == blockEnd) {
        out.push_back(std::string(1, c));
        s = "";
      } else {
        s += c;
      }
    } else if (s.size() > 0) {
      out.push_back(s);
      s = "";
    }
  }
  if (s.size() > 0) {
    out.push_back(s);
  }
  
  return out;
}

bool ShaderEditor::isAStatementIndent(std::string &word) {
  for (size_t i=0; i<(sizeof(statementIndent)/sizeof(statementIndent[0])); i++) {
    std::string &statementElem = statementIndent[i];
    if (statementElem == word) {
      return true;
    }
  }
  return false;
}

ShaderEditor::IndentationStatus ShaderEditor::GetIndentState(int line) {
  size_t i;
  IndentationStatus indentState = isNone;
  std::vector<std::string> controlWords = GetLinePartsInStyle(line, statementIndentStyleNumber);
  for (i=0; i<controlWords.size(); i++) {
    std::string &controlWord = controlWords[i];
    if (isAStatementIndent(controlWord)) {
      indentState = isKeyWordStart;
    }
  }
  
  controlWords = GetLinePartsInStyle(line, blockAndStatementEndStyleNumber);
  for (i=0; i<controlWords.size(); i++) {
    std::string &controlWord = controlWords[i];
    if (controlWord.size() < 1) continue;
    if (statementEnd == controlWord[0]) {
      indentState = isNone;
    }
  }
  
  for (i=0; i<controlWords.size(); i++) {
    std::string &controlWord = controlWords[i];
    if (controlWord.size() < 1) continue;
    if (blockEnd == controlWord[0]) {
      indentState = isBlockEnd;
    }
    if (blockStart == controlWord[0]) {
      indentState = isBlockStart;
    }
  }
  
  return indentState;
}

int ShaderEditor::IndentOfBlock(int line) {
  if (line < 0)
    return 0;
  int indentSize = WndProc(SCI_GETINDENT, 0, 0);
  int indentBlock = GetLineIndentation(line);
  int backLine = line;
  IndentationStatus indentState = isNone;

  int lineLimit = line - statementLookback;
  if (lineLimit < 0) lineLimit = 0;

  while ((backLine >= lineLimit) && (indentState == isNone)) {
    indentState = GetIndentState(backLine);
    if (indentState != isNone) {
      indentBlock = GetLineIndentation(backLine);
      if (indentState == isBlockStart) {
          indentBlock += indentSize;
      }
      if (indentState == isBlockEnd) {
        if (indentBlock < 0)
          indentBlock = 0;
      }
      if ((indentState == isKeyWordStart) && (backLine == line))
        indentBlock += indentSize;
    }
    backLine--;
  }
  
  return indentBlock;
}

bool ShaderEditor::RangeIsAllWhitespace(int start, int end) {
  for (int i = start; i < end; i++) {
    if ((WndProc(SCI_GETCHARAT, i, 0) != ' ') && (WndProc(SCI_GETCHARAT, i, 0) != '\t'))
      return false;
  }
  return true;
}

void ShaderEditor::AutomaticIndentation(char ch) {
  Sci_CharacterRange crange = GetSelection();
  int selStart = static_cast<int>(crange.cpMin);
  int curLine = GetCurrentLineNumber();
  int thisLineStart = WndProc(SCI_POSITIONFROMLINE, curLine, 0);
  int indentSize = WndProc(SCI_GETINDENT, 0, 0);
  int indentBlock = IndentOfBlock(curLine - 1);
 
  if (ch == blockEnd) {
    if (RangeIsAllWhitespace(thisLineStart, selStart - 1)) {
      SetLineIndentation(curLine, indentBlock - indentSize);
    }
  } else if (ch == blockStart) {
    if (GetIndentState(curLine - 1) == isKeyWordStart) {
      if (RangeIsAllWhitespace(thisLineStart, selStart - 1)) {
        SetLineIndentation(curLine, indentBlock - indentSize);
      }
    }
  } else if ((ch == '\r' || ch == '\n') && (selStart == thisLineStart)) {
    SetLineIndentation(curLine, indentBlock);
  }
}
