#include "../ShaderEditor.h"
#include "../Renderer.h"
#include "external/scintilla/lexlib/PropSetSimple.h"

ShaderEditor::ShaderEditor( Scintilla::Surface *s )
{
  surfaceWindow = s;
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

#define BACKGROUND(x) ( (x) | 0xC0000000 )

const size_t NB_FOLDER_STATE = 7;
const size_t FOLDER_TYPE = 0;

const int markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};

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

void ShaderEditor::Initialise()
{
  wMain = (Scintilla::WindowID)1234;

  lexState = new Scintilla::LexState( pdoc );

  WndProc( SCI_SETBUFFEREDDRAW, NULL, NULL );

  //WndProc( SCI_SETLEXERLANGUAGE, SCLEX_CPP, NULL );

  char * font = "Input-Regular_(InputMono-Medium).ttf";
  SetAStyle( STYLE_DEFAULT,     0xFFFFFFFF, BACKGROUND( 0x000000 ), 16, font);
  WndProc( SCI_STYLECLEARALL, NULL, NULL );
  SetAStyle( STYLE_INDENTGUIDE, 0xFFC0C0C0, BACKGROUND( 0x000000 ), 16, font);
  SetAStyle( STYLE_BRACELIGHT,  0xFF00FF00, BACKGROUND( 0x000000 ), 16, font);
  SetAStyle( STYLE_BRACEBAD,    0xFF0000FF, BACKGROUND( 0x000000 ), 16, font);
  SetAStyle( STYLE_LINENUMBER,  0xFFC0C0C0, BACKGROUND( 0x000000 ), 16, font);

  WndProc(SCI_SETFOLDMARGINCOLOUR,   1, BACKGROUND( 0x1A1A1A ));
  WndProc(SCI_SETFOLDMARGINHICOLOUR, 1, BACKGROUND( 0x1A1A1A ));
  WndProc(SCI_SETSELBACK,            1, BACKGROUND( 0xCC9966 ));

  WndProc(SCI_SETMARGINWIDTHN, 0, 44);//Calculate correct width
  WndProc(SCI_SETMARGINWIDTHN, 1, 20);//Calculate correct width
  WndProc(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);//Calculate correct width

  WndProc( SCI_SETCARETFORE,          0xFFFFFFFF, 0);
  WndProc( SCI_SETCARETLINEVISIBLE,   1, NULL);
  WndProc( SCI_SETCARETLINEBACK,      0xFFFFFFFF, NULL);
  WndProc( SCI_SETCARETLINEBACKALPHA, 0x20, NULL);

  for (int i = 0 ; i < NB_FOLDER_STATE ; i++)
  {
    WndProc(SCI_MARKERDEFINE, markersArray[FOLDER_TYPE][i], markersArray[4][i]);
    WndProc(SCI_MARKERSETBACK, markersArray[FOLDER_TYPE][i], 0xFF6A6A6A);
    WndProc(SCI_MARKERSETFORE, markersArray[FOLDER_TYPE][i], 0xFF333333);
  }
  WndProc(SCI_SETUSETABS, 1, NULL);
  WndProc(SCI_SETTABWIDTH, 4, NULL);
  WndProc(SCI_SETINDENTATIONGUIDES, SC_IV_REAL, NULL);

  lexState->SetLexer( SCLEX_CPP );

  SetAStyle(SCE_C_DEFAULT,      0xFFFFFFFF, BACKGROUND( 0x000000 ), 16, font);
  SetAStyle(SCE_C_WORD,         0xFF0066FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_WORD2,        0xFFFFFF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_PREPROCESSOR, 0xFFC0C0C0, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_NUMBER,       0xFF0080FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_OPERATOR,     0xFF00CCFF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENT,      0xFF00FF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENTLINE,  0xFF00FF00, BACKGROUND( 0x000000 ));

  lexState->Colourise( 0, -1 );

  //WndProc( SCI_COLOURISE, NULL, NULL );

  vs.Refresh( *surfaceWindow, 4 );
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

}

void ShaderEditor::Paste()
{

}

void ShaderEditor::ClaimSelection()
{

}

void ShaderEditor::NotifyChange()
{

}

void ShaderEditor::NotifyParent( Scintilla::SCNotification scn )
{

}

void ShaderEditor::CopyToClipboard( const Scintilla::SelectionText &selectedText )
{

}

void ShaderEditor::SetMouseCapture( bool on )
{

}

bool ShaderEditor::HaveMouseCapture()
{
  return false;
}

sptr_t ShaderEditor::DefWndProc( unsigned int iMessage, uptr_t wParam, sptr_t lParam )
{
  return 0;
}

void ShaderEditor::Paint()
{
  Renderer::SwitchToTextRenderingMode();
  int m = 10;
  Scintilla::Editor::Paint( surfaceWindow, GetClientRectangle() );
//  view.FormatRange( true, NULL, surfaceWindow, surfaceWindow, *this, &vs );
}

void ShaderEditor::SetText( char * buf )
{
  //char * buf = "test test test";
  WndProc( SCI_ADDTEXT, strlen(buf), (sptr_t)buf );
  SetFocusState( true );
  //GoToLine( 0 );
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

  int lengthDoc = WndProc( SCI_GETLENGTH, NULL, NULL );

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