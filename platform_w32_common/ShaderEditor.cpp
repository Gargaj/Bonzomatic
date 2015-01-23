#include "../ShaderEditor.h"
#include "../Renderer.h"

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

void ShaderEditor::Initialise()
{
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

  WndProc( SCI_SETCARETFORE,          0xFFFFFFFF, 0);
  WndProc( SCI_SETCARETLINEVISIBLE,   1, NULL);
  WndProc( SCI_SETCARETLINEBACK,      0xFFFFFFFF, NULL);
  WndProc( SCI_SETCARETLINEBACKALPHA, 0x20, NULL);

  SetAStyle(SCE_C_DEFAULT,      0xFFFFFFFF, BACKGROUND( 0x000000 ), 16, font);
  SetAStyle(SCE_C_WORD,         0xFF0066FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_WORD2,        0xFFFFFF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_PREPROCESSOR, 0xFFC0C0C0, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_NUMBER,       0xFF0080FF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_OPERATOR,     0xFF00CCFF, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENT,      0xFF00FF00, BACKGROUND( 0x000000 ));
  SetAStyle(SCE_C_COMMENTLINE,  0xFF00FF00, BACKGROUND( 0x000000 ));

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
  int m = 5;
  Scintilla::Editor::Paint( surfaceWindow, Scintilla::PRectangle(m,m,1280-m,720-m) );
  //Scintilla::Editor::FormatRange( true, NULL );
  //view.FormatRange( true, NULL, surfaceWindow
}

void ShaderEditor::SetText( char * buf )
{
  //char * buf = "test test test";
  WndProc( SCI_ADDTEXT, strlen(buf), (sptr_t)buf );
  SetFocusState( true );
}

void ShaderEditor::Tick()
{
  Scintilla::Editor::Tick();
}

void ShaderEditor::SetTicking( bool on )
{

}
