#include <d3d11.h>
#include <d3dx11.h>
#include <D3DX10math.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "CommonFont.h"

#include "CommonApp.h"
#include "D3DHelpers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static const int NUM_CHARS = 100;
static const int NUM_VTXS = NUM_CHARS * 4;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static HFONT CreateGDIFont(HDC hDC, const char *pFontName, int height, uint32_t fontCreateFlags)
{
	LOGFONT lf;

	lf.lfHeight = -MulDiv(height, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = fontCreateFlags & CommonFont::CREATE_BOLD ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision =	OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH;
	strncpy_s(lf.lfFaceName, sizeof lf.lfFaceName, pFontName, _TRUNCATE);

	HFONT hFont = CreateFontIndirect(&lf);
	return hFont;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const D3DXVECTOR2 CommonFont::Style::DEFAULT_SCALE(1.f, 1.f);
const VertexColour CommonFont::Style::DEFAULT_COLOUR(255, 255, 255, 255);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonFont::Style::Style():
scale(DEFAULT_SCALE),
colour(DEFAULT_COLOUR)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonFont::Style::Style(const VertexColour &colourArg):
scale(DEFAULT_SCALE),
colour(colourArg)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonFont::Style::Style(const VertexColour &colourArg, const D3DXVECTOR2 &scaleArg):
scale(scaleArg),
colour(colourArg)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct CommonFont::Glyph
{
	D3DXVECTOR2 size;
	D3DXVECTOR2 texMini;
	D3DXVECTOR2 texMaxi;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CommonFont::PaintAlphabet(HDC hDC, int width, int height, Glyph *pGlyphs)
{
	SIZE size;

	char ch = 'x';
	GetTextExtentPoint32(hDC, &ch, 1, &size);

	int spacing = int(ceilf(size.cy * .3f));

	int x = 0;
	int y = 0;
	LONG lineMaxHeight = 0;//height of tallest glyph on this line

	for (int ch = 32; ch < 127; ++ch)
	{
		char c = (char)ch;
		if (!GetTextExtentPoint32(hDC, &c, 1, &size))
		{
			size.cx = 0;
			size.cy = 0;
		}

		if (x + size.cx + spacing > width)
		{
			x = 0;
			y += lineMaxHeight + 1;//tm.tmHeight + 1;

			lineMaxHeight = 0;
		}

		if (y + size.cy > height)
			return false;

		if (size.cy > lineMaxHeight)
			lineMaxHeight = size.cy;

		if (pGlyphs)
		{
			ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, &c, 1, NULL);

			Glyph *pGlyph = &pGlyphs[ch - 32];

			pGlyph->texMini.x = x / float(width);
			pGlyph->texMini.y = y / float(height);

			pGlyph->texMaxi.x = (x + size.cx) / float(width);
			pGlyph->texMaxi.y = (y + size.cy) / float(height);

			pGlyph->size.x = float(size.cx);
			pGlyph->size.y = float(size.cy);
		}

		x += size.cx + spacing;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonFont *CommonFont::CreateByName(const char *pFontName, int height, uint32_t createFlags, CommonApp *pApp)
{
	HRESULT hr;

	HFONT hFont = NULL;

	HBITMAP hBitmap = NULL;

	HDC hDC = CreateCompatibleDC(NULL);
	SetMapMode(hDC, MM_TEXT);
	SaveDC(hDC);

	CommonFont *pFont = NULL;

	Glyph *pGlyphs = NULL;

	ID3D11Texture2D *pTexture = NULL;
	ID3D11ShaderResourceView *pTextureView = NULL;

	ID3D11Buffer *pVB = NULL;
	ID3D11Buffer *pIB = NULL;

	{
		// Try to create font.
		hFont = CreateGDIFont(hDC, pFontName, height, createFlags);
		if (!hFont)
			goto done;//font creation failed.

		SelectObject(hDC, hFont);

		// Decide on reasonable size for texture.
		int texWidth = 128, texHeight = 128;

		while (!PaintAlphabet(hDC, texWidth, texHeight, NULL))
		{
			texWidth *= 2;
			texHeight *= 2;

			if (texWidth == 4096 || texHeight == 4096)
				goto done;//font too large.
		}

		// Create GDI bitmap for texture.
		BITMAPINFO bmi;
		memset(&bmi, 0, sizeof bmi);

		bmi.bmiHeader.biSize = sizeof bmi.bmiHeader;
		bmi.bmiHeader.biWidth = texWidth;
		bmi.bmiHeader.biHeight = -texHeight;//-ve = top down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biBitCount = 32;

		DWORD *pDIBits;

		hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void **)&pDIBits, NULL, 0);
		if (!hBitmap)
			goto done;

		// Paint font into GDI bitmap and this time store off the texture
		// coordinates for each glyph.
		pGlyphs = new Glyph[96];

		SelectObject(hDC, hBitmap);

		SetTextColor(hDC, RGB(255, 255, 255));
		SetBkColor(hDC, RGB(0, 0, 0));
		SetTextAlign(hDC, TA_TOP);

		PaintAlphabet(hDC, texWidth, texHeight, pGlyphs);

		// Fix up bitmap data - the font shape should be in the alpha channel,
		// and the RGB channels should be all white.
		GdiFlush();

		for (int i = 0; i < texWidth * texHeight; ++i)
			pDIBits[i] = ((pDIBits[i] & 0xFF) << 24) | 0x00FFFFFF;

		// Create D3D texture for that.
		D3D11_TEXTURE2D_DESC td;

		td.Width = texWidth;
		td.Height = texHeight;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_IMMUTABLE;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA srd;

		srd.pSysMem = pDIBits;
		srd.SysMemPitch = texWidth * 4;
		srd.SysMemSlicePitch = 0;

		hr = pApp->GetDevice()->CreateTexture2D(&td, &srd, &pTexture);
		if (FAILED(hr))
			goto done;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;

		srvd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.MipLevels = 1;

		hr = pApp->GetDevice()->CreateShaderResourceView(pTexture, &srvd, &pTextureView);
		if (FAILED(hr))
			goto done;
	}

	// Create IB
	{
		uint16_t ibData[NUM_CHARS * 6];

		for (int i = 0; i < NUM_CHARS; ++i)
		{
			ibData[i * 6 + 0] = uint16_t(i * 4 + 0);
			ibData[i * 6 + 1] = uint16_t(i * 4 + 1);
			ibData[i * 6 + 2] = uint16_t(i * 4 + 2);

			ibData[i * 6 + 3] = uint16_t(i * 4 + 1);
			ibData[i * 6 + 4] = uint16_t(i * 4 + 3);
			ibData[i * 6 + 5] = uint16_t(i * 4 + 2);
		}

		pIB = CreateImmutableIndexBuffer(pApp->GetDevice(), sizeof ibData, ibData);
		if (!pIB)
			goto done;
	}

	// Create VB
	{
		pVB = CreateBuffer(pApp->GetDevice(), NUM_VTXS * sizeof(Vertex_Pos3fColour4ubTex2f), D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);
		if (!pVB)
			goto done;
	}

	// That seemed to work; collate it all and make a CommonFont.

	pFont = new CommonFont;

	pFont->m_pApp = pApp;

	pFont->m_pGlyphs = pGlyphs;
	pGlyphs = NULL;

	pFont->m_pTexture = pTexture;
	pTexture = NULL;

	pFont->m_pTextureView = pTextureView;
	pTextureView = NULL;

	pFont->m_pIB = pIB;
	pIB = NULL;

	pFont->m_pVB = pVB;
	pVB = NULL;

done:
	Release(pTextureView);
	Release(pTexture);
	Release(pVB);
	Release(pIB);

	if (hDC)
	{
		RestoreDC(hDC, -1);

		DeleteDC(hDC);
		hDC = NULL;
	}

	if (hBitmap)
	{
		DeleteObject(hBitmap);
		hBitmap = NULL;
	}

	if (hFont)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}

	delete[] pGlyphs;
	pGlyphs = NULL;

	return pFont;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CommonFont::CommonFont():
m_pApp(NULL),
m_pGlyphs(NULL),
m_pTexture(NULL),
m_pTextureView(NULL),
m_pIB(NULL),
m_pVB(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CommonFont::~CommonFont()
{
	Release(m_pVB);
	Release(m_pIB);

	Release(m_pTextureView);
	Release(m_pTexture);

	delete[] m_pGlyphs;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static const CommonFont::Style DEFAULT_STYLE;

void CommonFont::DrawString(const D3DXVECTOR3 &startPos, const Style *pStyle, const char *pStr)
{
	HRESULT hr;

	ID3D11SamplerState *pSamplerState = m_pApp->GetSamplerState(true);
	ID3D11DeviceContext *pContext = m_pApp->GetDeviceContext();
	Vertex_Pos3fColour4ubTex2f *pVtxs = NULL;
	int numChars = 0;

	m_pApp->SetRasterizerState(false);
	m_pApp->SetBlendState(true);

	D3DXVECTOR3 pos = startPos;

	// Use the default style if one wasn't specified.
	if (!pStyle)
		pStyle = &DEFAULT_STYLE;

	for (size_t chIdx = 0; pStr[chIdx] != 0; ++chIdx)
	{
		int c = pStr[chIdx];

		if (c < 32 || c >= 127)
			continue;//can't print this char

		const Glyph *pGlyph = &m_pGlyphs[c - 32];

		if (numChars >= NUM_CHARS)
		{
			assert(numChars == NUM_CHARS);

			pContext->Unmap(m_pVB, 0);

			m_pApp->DrawTextured(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pVB, m_pIB, numChars * 6, m_pTextureView, pSamplerState);

			pVtxs = NULL;
			numChars = 0;
		}

		if (!pVtxs)
		{
			D3D11_MAPPED_SUBRESOURCE ms;
			hr = pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
			if (FAILED(hr))
			{
				// erm...
				return;
			}

			pVtxs = static_cast<Vertex_Pos3fColour4ubTex2f *>(ms.pData);
			numChars = 0;
		}

		Vertex_Pos3fColour4ubTex2f *pVtx = &pVtxs[numChars * 4];
		++numChars;

		pVtx->pos.x = pos.x;
		pVtx->pos.y = pos.y;
		pVtx->pos.z = pos.z;
		pVtx->colour = pStyle->colour;
		pVtx->tex.x = pGlyph->texMini.x;
		pVtx->tex.y = pGlyph->texMaxi.y;
		++pVtx;

		pVtx->pos.x = pos.x + pGlyph->size.x * pStyle->scale.x;
		pVtx->pos.y = pos.y;
		pVtx->pos.z = pos.z;
		pVtx->colour = pStyle->colour;
		pVtx->tex.x = pGlyph->texMaxi.x;
		pVtx->tex.y = pGlyph->texMaxi.y;
		++pVtx;

		pVtx->pos.x = pos.x;
		pVtx->pos.y = pos.y + pGlyph->size.y * pStyle->scale.y;
		pVtx->pos.z = pos.z;
		pVtx->colour = pStyle->colour;
		pVtx->tex.x = pGlyph->texMini.x;
		pVtx->tex.y = pGlyph->texMini.y;
		++pVtx;

		pVtx->pos.x = pos.x + pGlyph->size.x * pStyle->scale.x;
		pVtx->pos.y = pos.y + pGlyph->size.y * pStyle->scale.y;
		pVtx->pos.z = pos.z;
		pVtx->colour = pStyle->colour;
		pVtx->tex.x = pGlyph->texMaxi.x;
		pVtx->tex.y = pGlyph->texMini.y;
		++pVtx;

		pos.x += pGlyph->size.x * pStyle->scale.x;
	}

	if (pVtxs)
		pContext->Unmap(m_pVB, 0);

	if (numChars > 0)
		m_pApp->DrawTextured(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pVB, m_pIB, numChars * 6, m_pTextureView, pSamplerState);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CommonFont::DrawStringf(const D3DXVECTOR3 &pos, const Style *pStyle, const char *pFmt, ...)
{
	char aStr[1000];

	va_list v;
	va_start(v, pFmt);
	_vsnprintf_s(aStr, sizeof aStr, _TRUNCATE, pFmt, v);
	va_end(v);

	this->DrawString(pos, pStyle, aStr);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
