#ifndef HEADER_3325B9D7B7774F50B1D7AE51B29087B5
#define HEADER_3325B9D7B7774F50B1D7AE51B29087B5

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class CommonApp;

#include <stdint.h>
#include "D3DHelpers.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CommonFont
{
public:
	enum CreateFlags
	{
		CREATE_BOLD = 1,
	};

	struct Style
	{
		static const D3DXVECTOR2 DEFAULT_SCALE;//(1,1).
		D3DXVECTOR2 scale;

		static const VertexColour DEFAULT_COLOUR;//white
		VertexColour colour;

		// Any members not set by a particular constructor are set to
		// their corresponding default values - hopefully in an
		// obvious way...
		Style();
		explicit Style(const VertexColour &colour);
		Style(const VertexColour &colour, const D3DXVECTOR2 &scale);
	};

	static CommonFont *CreateByName(const char *pFontName, int height, uint32_t createFlags, CommonApp *pApp);

	~CommonFont();

	// If pStyle is NULL, a default style will be used.
	//
	// Text is drawn in world space starting at `pos' then moving along
	// the positive X axis, with the positive Y axis being up:
	//
	//  Y Axis
	//    ^
	//    | TEXT
	//    +------> X Axis
	// 
	// If drawn at a scale of (1,1), 1 pixel in the font is equivalent to 1
	// world space unit.
	void DrawString(const D3DXVECTOR3 &pos, const Style *pStyle, const char *pStr);
	void DrawStringf(const D3DXVECTOR3 &pos, const Style *pStyle, const char *pFmt, ...);
protected:
private:
	CommonApp *m_pApp;

	struct Glyph;
	Glyph *m_pGlyphs;

	ID3D11Texture2D *m_pTexture;
	ID3D11ShaderResourceView *m_pTextureView;

	ID3D11Buffer *m_pVB, *m_pIB;

	static bool PaintAlphabet(HDC hDC, int width, int height, Glyph *pGlyphs);

	CommonFont();

	CommonFont(const CommonFont &);
	CommonFont &operator=(const CommonFont &);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif//HEADER_3325B9D7B7774F50B1D7AE51B29087B5
