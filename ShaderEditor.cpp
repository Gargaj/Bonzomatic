#include "../ShaderEditor.h"
#include "../Renderer.h"
#include "external/scintilla/lexlib/PropSetSimple.h"
#include "Clipboard.h"

ShaderEditor::ShaderEditor( Scintilla::Surface *s )
{
  bReadOnly = false;
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
const char glslKeyword[] =
  "discard struct if else switch case default break goto return for while do continue";

const char glslType[] = 
  "attribute const in inout out uniform varying invariant "
  "centroid flat smooth noperspective layout patch sample "
  "subroutine lowp mediump highp precision "
  "void float vec2 vec3 vec4 bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 "
  "uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 "
  "sampler1D sampler2D sampler3D isampler2D isampler1D isampler3D "
  "usampler1D usampler2D usampler3D "
  "sampler1DShadow sampler2DShadow sampler1DArray sampler2DArray "
  "sampler1DArrayShadow sampler2DArrayShadow "
  "samplerCube samperCubeShadow samperCubeArrayShadow ";

const char glslBuiltin[] = 
  "radians degrees sin cos tan asin acos atan sinh "
  "cosh tanh asinh acosh atanh pow exp log exp2 "
  "log2 sqrt inversesqrt abs sign floor trunc round "
  "roundEven ceil fract mod modf min max clamp mix "
  "step smoothstep isnan isinf floatBitsToInt floatBitsToUint "
  "intBitsToFloat uintBitsToFloat fma frexp ldexp packUnorm2x16 "
  "packUnorm4x8 packSnorm4x8 unpackUnorm2x16 unpackUnorm4x8 "
  "unpackSnorm4x8 packDouble2x32 unpackDouble2x32 length distance "
  "dot cross normalize ftransform faceforward reflect "
  "refract matrixCompMult outerProduct transpose determinant "
  "inverse lessThan lessThanEqual greaterThan greaterThanEqual "
  "equal notEqual any all not uaddCarry usubBorrow "
  "umulExtended imulExtended bitfieldExtract bitfildInsert "
  "bitfieldReverse bitCount findLSB findMSB textureSize "
  "textureQueryLOD texture textureProj textureLod textureOffset "
  "texelFetch texelFetchOffset textureProjOffset textureLodOffset "
  "textureProjLod textureProjLodOffset textureGrad textureGradOffset "
  "textureProjGrad textureProjGradOffset textureGather "
  "textureGatherOffset texture1D texture2D texture3D texture1DProj "
  "texture2DProj texture3DProj texture1DLod texture2DLod "
  "texture3DLod texture1DProjLod texture2DProjLod texture3DProjLod "
  "textureCube textureCubeLod shadow1D shadow2D shadow1DProj "
  "shadow2DProj shadow1DLod shadow2DLod shadow1DProjLod "
  "shadow2DProjLod dFdx dFdy fwidth interpolateAtCentroid "
  "interpolateAtSample interpolateAtOffset noise1 noise2 noise3 "
  "noise4 EmitStreamVertex EndStreamPrimitive EmitVertex "
  "EndPrimitive barrier "
  "gl_VertexID gl_InstanceID gl_Position gl_PointSize "
  "gl_ClipDistance gl_PrimitiveIDIn gl_InvocationID gl_PrimitiveID "
  "gl_Layer gl_PatchVerticesIn gl_TessLevelOuter gl_TessLevelInner "
  "gl_TessCoord gl_FragCoord gl_FrontFacing gl_PointCoord "
  "gl_SampleID gl_SamplePosition gl_FragColor gl_FragData "
  "gl_FragDepth gl_SampleMask gl_ClipVertex gl_FrontColor "
  "gl_BackColor gl_FrontSecondaryColor gl_BackSecondaryColor "
  "gl_TexCoord gl_FogFragCoord gl_Color gl_SecondaryColor "
  "gl_Normal gl_Vertex gl_MultiTexCoord0 gl_MultiTexCoord1 "
  "gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 "
  "gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_FogCoord "
  "gl_MaxVertexAttribs gl_MaxVertexUniformComponents gl_MaxVaryingFloats "
  "gl_MaxVaryingComponents gl_MaxVertexOutputComponents "
  "gl_MaxGeometryInputComponents gl_MaxGeometryOutputComponents "
  "gl_MaxFragmentInputComponents gl_MaxVertexTextureImageUnits "
  "gl_MaxCombinedTextureImageUnits gl_MaxTextureImageUnits "
  "gl_MaxFragmentUniformComponents gl_MaxDrawBuffers gl_MaxClipDistances "
  "gl_MaxGeometryTextureImageUnits gl_MaxGeometryOutputVertices "
  "gl_MaxGeometryTotalOutputComponents gl_MaxGeometryUniformComponents "
  "gl_MaxGeometryVaryingComponents gl_MaxTessControlInputComponents "
  "gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits "
  "gl_MaxTessControlUniformComponents "
  "gl_MaxTessControlTotalOutputComponents "
  "gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents "
  "gl_MaxTessEvaluationTextureImageUnits "
  "gl_MaxTessEvaluationUniformComponents gl_MaxTessPatchComponents "
  "gl_MaxPatchVertices gl_MaxTessGenLevel gl_MaxTextureUnits "
  "gl_MaxTextureCoords gl_MaxClipPlanes "
  "gl_DepthRange gl_ModelViewMatrix gl_ProjectionMatrix "
  "gl_ModelViewProjectionMatrix gl_TextureMatrix gl_NormalMatrix "
  "gl_ModelViewMatrixInverse gl_ProjectionMatrixInverse "
  "gl_ModelViewProjectionMatrixInverse gl_TextureMatrixInverse "
  "gl_ModelViewMatrixTranspose gl_ProjectionMatrixTranspose "
  "gl_ModelViewProjectionMatrixTranspose gl_TextureMatrixTranspose "
  "gl_ModelViewMatrixInverseTranspose gl_ProjectionMatrixInverseTranspose "
  "gl_ModelViewProjectionMatrixInverseTranspose "
  "gl_TextureMatrixInverseTranspose gl_NormalScale gl_ClipPlane "
  "gl_Point gl_FrontMaterial gl_BackMaterial gl_LightSource "
  "gl_LightModel gl_FrontLightModelProduct gl_BackLightModelProduct "
  "gl_FrontLightProduct gl_BackLightProduct gl_TextureEnvColor "
  "gl_EyePlaneS gl_EyePlaneT gl_EyePlaneR gl_EyePlaneQ "
  "gl_ObjectPlaneS gl_ObjectPlaneT gl_ObjectPlaneR gl_ObjectPlaneQ "
  "gl_Fog";

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
  wMain = (Scintilla::WindowID)(wndID++);

  lexState = new Scintilla::LexState( pdoc );

  WndProc( SCI_SETBUFFEREDDRAW, NULL, NULL );
  WndProc( SCI_SETCODEPAGE, SC_CP_UTF8, NULL );

  //WndProc( SCI_SETLEXERLANGUAGE, SCLEX_CPP, NULL );

  char * font = "ProFontWindows.ttf";
  int fontSize = 16;
  SetAStyle( STYLE_DEFAULT,     0xFFFFFFFF, BACKGROUND( 0x000000 ), fontSize, font);
  WndProc( SCI_STYLECLEARALL, NULL, NULL );
  SetAStyle( STYLE_LINENUMBER,  0xFFC0C0C0, BACKGROUND( 0x000000 ), fontSize, font);
  SetAStyle( STYLE_BRACELIGHT,  0xFF00FF00, BACKGROUND( 0x000000 ), fontSize, font);
  SetAStyle( STYLE_BRACEBAD,    0xFF0000FF, BACKGROUND( 0x000000 ), fontSize, font);
  SetAStyle( STYLE_INDENTGUIDE, 0xFFC0C0C0, BACKGROUND( 0x000000 ), fontSize, font);

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
  WndProc(SCI_SETUSETABS, 1, NULL);
  WndProc(SCI_SETTABWIDTH, 4, NULL);
  WndProc(SCI_SETINDENTATIONGUIDES, SC_IV_REAL, NULL);

  lexState->SetLexer( SCLEX_CPP );
  lexState->SetWordList(0, glslKeyword);
  lexState->SetWordList(1, glslType);
  lexState->SetWordList(4, glslBuiltin);

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

void ShaderEditor::Initialise( PRectangle rect )
{
  Initialise();
  wMain.SetPosition(rect);
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
  char * p = new char[n + 1];
  memset(p,0,n+1);
  Clipboard::GetContents( p, n );

  InsertPasteShape(p, n, pasteStream);

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

}

void ShaderEditor::CopyToClipboard( const Scintilla::SelectionText &selectedText )
{
  Clipboard::Copy( selectedText.Data(), selectedText.Length() );
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
  Renderer::SetTextRenderingViewport( wMain.GetPosition() );
  Scintilla::Editor::Paint( surfaceWindow, GetClientRectangle() );
}

void ShaderEditor::SetText( char * buf )
{
  WndProc( SCI_SETREADONLY, false, NULL );
  WndProc( SCI_CLEARALL, false, NULL );
  WndProc( SCI_SETUNDOCOLLECTION, 0, NULL);
  WndProc( SCI_ADDTEXT, strlen(buf), (sptr_t)buf );
  WndProc( SCI_SETUNDOCOLLECTION, 1, NULL);
  WndProc( SCI_SETREADONLY, bReadOnly, NULL );
  WndProc( SCI_GOTOPOS, 0, NULL );
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

void ShaderEditor::SetReadOnly( bool b )
{
  bReadOnly = b;
  WndProc( SCI_SETREADONLY, bReadOnly, NULL );
  if (bReadOnly)
  {
    WndProc(SCI_SETMARGINWIDTHN, 0, 0);
    WndProc(SCI_SETMARGINWIDTHN, 1, 0);
    WndProc( SCI_SETCARETLINEVISIBLE,   0, NULL);
    WndProc( SCI_SETCARETFORE,          0, 0);
  }
  else
  {
    WndProc(SCI_SETMARGINWIDTHN, 0, 44);//Calculate correct width
    WndProc(SCI_SETMARGINWIDTHN, 1, 20);//Calculate correct width
    WndProc(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);//Calculate correct width

    WndProc( SCI_SETCARETFORE,          0xFFFFFFFF, 0);
    WndProc( SCI_SETCARETLINEVISIBLE,   1, NULL);
    WndProc( SCI_SETCARETLINEBACK,      0xFFFFFFFF, NULL);
    WndProc( SCI_SETCARETLINEBACKALPHA, 0x20, NULL);
  }
}

void ShaderEditor::ButtonDown( Scintilla::Point pt, unsigned int curTime, bool shift, bool ctrl, bool alt )
{
  Scintilla::PRectangle rect = wMain.GetPosition();
  pt.x -= rect.left;
  pt.y -= rect.top;
  Scintilla::Editor::ButtonDown( pt, curTime, shift, ctrl, alt );
}
