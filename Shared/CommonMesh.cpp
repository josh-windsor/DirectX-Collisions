#define D3D_DEBUG_INFO
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "CommonApp.h"
#include "CommonMesh.h"

#include <assert.h>

#include <algorithm>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static IDirect3DDevice9 *CreateDevice9()
{
	// Start D3D
	//
	// This uses the D3D reference device. Ideally, it would use the
	// null reference device, which is for situations like this one
	// where rendering isn't necessary - but that doesn't seem to
	// work properly with PIX.
	//
	// In any event, the reference device works fine without having
	// to provide a window to render to, which is the most useful
	// thing.

	IDirect3D9 *pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pDirect3D9)
	{
		dprintf("%s: Direct3DCreate9 failed.\n", __FUNCTION__);
		return NULL;
	}

	D3DPRESENT_PARAMETERS pp;

	pp.AutoDepthStencilFormat=D3DFMT_D24S8;
	pp.BackBufferCount=1;
	pp.BackBufferFormat=D3DFMT_A8R8G8B8;
	pp.BackBufferHeight=256;//random choice
	pp.BackBufferWidth=256;//random choice
	pp.EnableAutoDepthStencil=TRUE;
	pp.Flags=0;
	pp.FullScreen_RefreshRateInHz=0;
	pp.hDeviceWindow=0;
	pp.MultiSampleQuality=0;
	pp.MultiSampleType=D3DMULTISAMPLE_NONE;
	pp.PresentationInterval=1;
	pp.SwapEffect=D3DSWAPEFFECT_COPY;
	pp.Windowed=TRUE;

	IDirect3DDevice9 *pDevice9 = NULL;
	HRESULT hr = pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, NULL,
		D3DCREATE_FPU_PRESERVE|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES|D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&pp, &pDevice9);
	if (FAILED(hr))
	{
		dprintf("%s:  CreateDevice failed.\n", __FUNCTION__);
		pDevice9 = NULL;
	}

	Release(pDirect3D9);

	return pDevice9;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct CommonMesh::Subset
{
	CommonApp::Shader *pShader;

	unsigned firstItem;
	unsigned numItems;

	ID3D11Buffer *pVertexBuffer;
	size_t vtxStride;

	ID3D11Buffer *pIndexBuffer;

	ID3D11Texture2D *pTexture;
	ID3D11ShaderResourceView *pTextureView;
	ID3D11SamplerState *pSamplerState;

	D3DXVECTOR3 localAABBMin;
	D3DXVECTOR3 localAABBMax;

	Subset();
	~Subset();
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh::Subset::Subset():
pShader(NULL),
firstItem(0),
numItems(0),
pVertexBuffer(NULL),
vtxStride(0),
pIndexBuffer(NULL),
pTexture(NULL),
pTextureView(NULL),
pSamplerState(NULL),
localAABBMin(0.f, 0.f, 0.f),
localAABBMax(0.f, 0.f, 0.f)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh::Subset::~Subset()
{
	Release(this->pVertexBuffer);
	Release(this->pIndexBuffer);
	Release(this->pTexture);
	Release(this->pTextureView);
	Release(this->pSamplerState);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static void CopyColour(VertexColour *pDest, const char *pSrcVertex, const D3DVERTEXELEMENT9 *pSrcElem, const D3DXMATERIAL *pMaterial9)
{
	D3DXCOLOR colour(1.f, 1.f, 1.f, 1.f);

	if (pSrcElem)
	{
		const D3DCOLOR *pSrc = reinterpret_cast<const D3DCOLOR *>(pSrcVertex + pSrcElem->Offset);

		colour.b *= *pSrc++ / 255.f;
		colour.g *= *pSrc++ / 255.f;
		colour.r *= *pSrc++ / 255.f;
		colour.a *= *pSrc++ / 255.f;
	}

	if (pMaterial9)
	{
		colour.r *= pMaterial9->MatD3D.Diffuse.r;
		colour.g *= pMaterial9->MatD3D.Diffuse.g;
		colour.b *= pMaterial9->MatD3D.Diffuse.b;

		// It's not quite clear what D3DX does with this. The docs suggest
		// that material diffuse alpha multiplies through, but that doesn't
		// seem to happen with D3DX meshes...

		//colour.a *= pMaterial9->MatD3D.Diffuse.a;
	}

	*pDest = VertexColour(UINT(colour));
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::LoadFromXFile(CommonApp *pApp, const char *pFileName)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	ID3DXBuffer *pMaterialsBuffer9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXLoadMeshFromX(pFileName, D3DXMESH_SYSTEMMEM, pDevice9, NULL, &pMaterialsBuffer9, NULL, NULL, &pMesh9);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, pMaterialsBuffer9);

done:
	Release(pMaterialsBuffer9);
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::NewBoxMesh(CommonApp *pApp, float width, float height, float depth)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXCreateBox(pDevice9, width, height, depth, &pMesh9, NULL);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, NULL);

done:
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::NewCylinderMesh(CommonApp *pApp, float radius1, float radius2, float length, unsigned slices, unsigned stacks)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXCreateCylinder(pDevice9, radius1, radius2, length, slices, stacks, &pMesh9, NULL);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, NULL);

done:
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::NewSphereMesh(CommonApp *pApp, float radius, unsigned slices, unsigned stacks)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXCreateSphere(pDevice9, radius, slices, stacks, &pMesh9, NULL);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, NULL);

done:
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::NewTorusMesh(CommonApp *pApp, float innerRadius, float outerRadius, unsigned sides, unsigned rings)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXCreateTorus(pDevice9, innerRadius, outerRadius, sides, rings, &pMesh9, NULL);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, NULL);

done:
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh *CommonMesh::NewTeapotMesh(CommonApp *pApp)
{
	HRESULT hr;
	IDirect3DDevice9 *pDevice9 = NULL;
	ID3DXMesh *pMesh9 = NULL;
	CommonMesh *pMesh = NULL;

	pDevice9 = CreateDevice9();
	if (!pDevice9)
		goto done;

	hr = D3DXCreateTeapot(pDevice9, &pMesh9, NULL);
	if (FAILED(hr))
		goto done;

	pMesh = ConvertFromD3DXMesh(pApp, pMesh9, NULL);

done:
	Release(pMesh9);
	Release(pDevice9);

	return pMesh;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh::CommonMesh():
m_pSubsets(NULL),
m_numSubsets(0),
m_pApp(NULL)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonMesh::~CommonMesh()
{
	delete[] m_pSubsets;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonMesh::Draw()
{
	for (size_t i = 0; i < m_numSubsets; ++i)
		this->DrawSubset(i);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

size_t CommonMesh::GetNumSubsets() const
{
	return m_numSubsets;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader *CommonMesh::GetSubsetShader(size_t subsetIndex) const
{
	if (subsetIndex >= m_numSubsets)
		return NULL;

	return m_pSubsets[subsetIndex].pShader;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonMesh::SetSubsetShader(size_t subsetIndex, CommonApp::Shader *pShader)
{
	if (subsetIndex >= m_numSubsets)
		return;

	m_pSubsets[subsetIndex].pShader = pShader;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonMesh::DrawSubset(size_t subsetIndex)
{
	if (subsetIndex >= m_numSubsets)
		return;

	const Subset *pSubset = &m_pSubsets[subsetIndex];

	m_pApp->DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, pSubset->pVertexBuffer, pSubset->vtxStride, pSubset->pIndexBuffer, pSubset->firstItem, 
		pSubset->numItems, pSubset->pTextureView, pSubset->pSamplerState, pSubset->pShader);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CommonMesh::GetSubsetLocalAABB(size_t subsetIndex, D3DXVECTOR3 *pLocalAABBMin, D3DXVECTOR3 *pLocalAABBMax) const
{
	assert(subsetIndex < m_numSubsets);

	const Subset *pSubset = &m_pSubsets[subsetIndex];

	*pLocalAABBMin = pSubset->localAABBMin;
	*pLocalAABBMax = pSubset->localAABBMax;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CommonMesh::SetShaderForAllSubsets(CommonApp::Shader *pShader)
{
	for (size_t i = 0; i < this->GetNumSubsets(); ++i)
		this->SetSubsetShader(i, pShader);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static void UpdateLocalAABB(D3DXVECTOR3 *pAABBMin, D3DXVECTOR3 *pAABBMax, DWORD i, const D3DXVECTOR3 &pos)
{
	if (i == 0)
	{
		*pAABBMin = pos;
		*pAABBMax = pos;
	}
	else
	{
		pAABBMin->x = std::min(pAABBMin->x, pos.x);
		pAABBMin->y = std::min(pAABBMin->y, pos.y);
		pAABBMin->z = std::min(pAABBMin->z, pos.z);

		pAABBMax->x = std::max(pAABBMax->x, pos.x);
		pAABBMax->y = std::max(pAABBMax->y, pos.y);
		pAABBMax->z = std::max(pAABBMax->z, pos.z);
	}
}

CommonMesh *CommonMesh::ConvertFromD3DXMesh(CommonApp *pApp, ID3DXMesh *pMesh9, ID3DXBuffer *pMaterialsBuffer9)
{
	CommonMesh *pResult = new CommonMesh;
	pResult->m_pApp = pApp;

	const D3DVERTEXELEMENT9 *pMeshPos9 = NULL;
	const D3DVERTEXELEMENT9 *pMeshColour9 = NULL;
	const D3DVERTEXELEMENT9 *pMeshNormal9 = NULL;
	const D3DVERTEXELEMENT9 *pMeshTexCoord9 = NULL;

	D3DXATTRIBUTERANGE *pRanges9 = NULL;
	DWORD numRanges9 = 0;

	// Get VB layout.
	{
		static const D3DVERTEXELEMENT9 DECL_END = D3DDECL_END();
		D3DVERTEXELEMENT9 aElements9[MAX_FVF_DECL_SIZE];
		pMesh9->GetDeclaration(aElements9);

		for (const D3DVERTEXELEMENT9 *pElem = aElements9; memcmp(pElem, &DECL_END, sizeof(D3DVERTEXELEMENT9)) != 0; ++pElem)
		{
			if (pElem->Stream == 0)
			{
				if (pElem->Usage == D3DDECLUSAGE_POSITION && pElem->UsageIndex == 0 && pElem->Type == D3DDECLTYPE_FLOAT3)
					pMeshPos9 = pElem;
				else if (pElem->Usage == D3DDECLUSAGE_COLOR && pElem->UsageIndex == 0 && pElem->Type == D3DDECLTYPE_D3DCOLOR)
					pMeshColour9 = pElem;
				else if (pElem->Usage == D3DDECLUSAGE_NORMAL && pElem->UsageIndex == 0 && pElem->Type == D3DDECLTYPE_FLOAT3)
					pMeshNormal9 = pElem;
				else if (pElem->Usage == D3DDECLUSAGE_TEXCOORD && pElem->UsageIndex == 0 && pElem->Type == D3DDECLTYPE_FLOAT2)
					pMeshTexCoord9 = pElem;
			}
		}
	}

	// Get attribute ranges.
	{
		pMesh9->GetAttributeTable(NULL, &numRanges9);

		if (numRanges9 == 0)
		{
			// Make one up...
			pRanges9 = new D3DXATTRIBUTERANGE[1];

			pRanges9[0].AttribId = 0;
			pRanges9[0].FaceStart = 0;
			pRanges9[0].FaceCount = pMesh9->GetNumFaces();
			pRanges9[0].VertexStart = 0;
			pRanges9[0].VertexCount = pMesh9->GetNumVertices();

			numRanges9 = 1;
		}
		else
		{
			pRanges9 = new D3DXATTRIBUTERANGE[numRanges9];
			pMesh9->GetAttributeTable(pRanges9, &numRanges9);
		}
	}

	// Convert subsets.
	{
		pResult->m_pSubsets = new Subset[numRanges9];
		pResult->m_numSubsets = 0;

		const D3DXMATERIAL *pMaterials9 = NULL;
		if (pMaterialsBuffer9)
			pMaterials9 = static_cast<D3DXMATERIAL *>(pMaterialsBuffer9->GetBufferPointer());

		for (DWORD subsetIdx = 0; subsetIdx < numRanges9; ++subsetIdx)
		{
			const D3DXATTRIBUTERANGE *pRange9 = &pRanges9[subsetIdx];
			const D3DXMATERIAL *pMaterial9 = NULL;
			if (pMaterials9)
				pMaterial9 = &pMaterials9[subsetIdx];

			//pSubset->firstItem = pRange9->FaceStart * 3;
			//pSubset->numItems = pRange9->FaceCount * 3;

			// Load material texture, if any.
			ID3D11Texture2D *pTexture = NULL;
			ID3D11ShaderResourceView *pTextureView = NULL;
			ID3D11SamplerState *pSamplerState = NULL;

			bool needsTexCoords = false;

			if (pMaterial9)
			{
				if (pMaterial9->pTextureFilename && strlen(pMaterial9->pTextureFilename) > 0)
				{
					LoadTextureFromFile(pResult->m_pApp->GetDevice(), pMaterial9->pTextureFilename, &pTexture, &pTextureView, &pSamplerState);
					needsTexCoords = true;
				}
			}

			if (!needsTexCoords)
				pMeshTexCoord9 = NULL;// in case there were some in the file anyway for some reason.

			// Get elements for this subset, ignoring any that are
			// actually inappropriate.

			const D3DVERTEXELEMENT9 *pPos9 = pMeshPos9;
			const D3DVERTEXELEMENT9 *pColour9 = pMeshColour9;
			const D3DVERTEXELEMENT9 *pNormal9 = pMeshNormal9;
			const D3DVERTEXELEMENT9 *pTexCoord9 = pMeshTexCoord9;

			// Copy appropriate part of vertex buffer.
			ID3D11Buffer *pVertexBuffer = NULL;
			size_t vtxStride11 = 0;
			CommonApp::Shader *pShader = NULL;
			D3DXVECTOR3 localAABBMin(0.f, 0.f, 0.f);
			D3DXVECTOR3 localAABBMax(0.f, 0.f, 0.f);

			{
				IDirect3DVertexBuffer9 *pMeshVB9;
				pMesh9->GetVertexBuffer(&pMeshVB9);

				DWORD vtxStride9 = pMesh9->GetNumBytesPerVertex();

				char *pSrc;
				pMeshVB9->Lock(0, 0, (void **)&pSrc, D3DLOCK_READONLY);
				pSrc += pRange9->VertexStart * vtxStride9;

				void *pInitData11 = NULL;

				if (pPos9 && !pNormal9 && !pTexCoord9)
				{
					Vertex_Pos3fColour4ub *pDest; 

					pInitData11 = pDest = static_cast<Vertex_Pos3fColour4ub *>(calloc(pRange9->VertexCount, sizeof *pDest));

					for (DWORD i = 0; i < pRange9->VertexCount; ++i, ++pDest, pSrc += vtxStride9)
					{
						pDest->pos = *reinterpret_cast<D3DXVECTOR3 *>(pSrc + pPos9->Offset);
						UpdateLocalAABB(&localAABBMin, &localAABBMax, i, pDest->pos);

						CopyColour(&pDest->colour, pSrc, pColour9, pMaterial9);
					}

					vtxStride11 = sizeof *pDest;
					pShader = pApp->GetUntexturedShader();
				}
				else if (pPos9 && pNormal9 && !pTexCoord9)
				{
					Vertex_Pos3fColour4ubNormal3f *pDest; 

					pInitData11 = pDest = static_cast<Vertex_Pos3fColour4ubNormal3f *>(calloc(pRange9->VertexCount, sizeof *pDest));

					for (DWORD i = 0; i < pRange9->VertexCount; ++i, ++pDest, pSrc += vtxStride9)
					{
						pDest->pos = *reinterpret_cast<D3DXVECTOR3 *>(pSrc + pPos9->Offset);
						UpdateLocalAABB(&localAABBMin, &localAABBMax, i, pDest->pos);

						CopyColour(&pDest->colour, pSrc, pColour9, pMaterial9);

						D3DXVec3Normalize(&pDest->normal, reinterpret_cast<D3DXVECTOR3 *>(pSrc + pNormal9->Offset));
					}

					vtxStride11 = sizeof *pDest;
					pShader = pApp->GetUntexturedLitShader();
				}
				else if (pPos9 && !pNormal9 && pTexCoord9)
				{
					Vertex_Pos3fColour4ubTex2f *pDest; 

					pInitData11 = pDest = static_cast<Vertex_Pos3fColour4ubTex2f *>(calloc(pRange9->VertexCount, sizeof *pDest));

					for (DWORD i = 0; i < pRange9->VertexCount; ++i, ++pDest, pSrc += vtxStride9)
					{
						pDest->pos = *reinterpret_cast<D3DXVECTOR3 *>(pSrc + pPos9->Offset);
						UpdateLocalAABB(&localAABBMin, &localAABBMax, i, pDest->pos);

						CopyColour(&pDest->colour, pSrc, pColour9, pMaterial9);

						pDest->tex = *reinterpret_cast<D3DXVECTOR2 *>(pSrc + pTexCoord9->Offset);
					}

					vtxStride11 = sizeof *pDest;
					pShader = pApp->GetTexturedShader();
				}
				else if (pPos9 && pNormal9 && pTexCoord9)
				{
					Vertex_Pos3fColour4ubNormal3fTex2f *pDest; 

					pInitData11 = pDest = static_cast<Vertex_Pos3fColour4ubNormal3fTex2f *>(calloc(pRange9->VertexCount, sizeof *pDest));

					for (DWORD i = 0; i < pRange9->VertexCount; ++i, ++pDest, pSrc += vtxStride9)
					{
						pDest->pos = *reinterpret_cast<D3DXVECTOR3 *>(pSrc + pPos9->Offset);
						UpdateLocalAABB(&localAABBMin, &localAABBMax, i, pDest->pos);

						CopyColour(&pDest->colour, pSrc, pColour9, pMaterial9);

						D3DXVec3Normalize(&pDest->normal, reinterpret_cast<D3DXVECTOR3 *>(pSrc + pNormal9->Offset));

						pDest->tex = *reinterpret_cast<D3DXVECTOR2 *>(pSrc + pTexCoord9->Offset);
					}

					vtxStride11 = sizeof *pDest;
					pShader = pApp->GetTexturedLitShader();
				}
				else
				{
					// The conversion code doesn't try to handle every
					// possible layout, just those corresponding to the
					// vertex types used by the CommonApp shaders.
				}

				pMeshVB9->Unlock();
				Release(pMeshVB9);

				if (pInitData11)
				{
					pVertexBuffer = CreateImmutableVertexBuffer(pResult->m_pApp->GetDevice(), pRange9->VertexCount * vtxStride11, pInitData11);

					free(pInitData11);
					pInitData11 = NULL;
				}
			}

			// Copy appropriate part of index buffer.
			ID3D11Buffer *pIndexBuffer = NULL;
			{
				IDirect3DIndexBuffer9 *pMeshIB9;
				pMesh9->GetIndexBuffer(&pMeshIB9);

				uint16_t *pIBData;
				pMeshIB9->Lock(0, 0, (void **)&pIBData, D3DLOCK_READONLY);

				uint16_t *pNewIBData = new uint16_t[pRange9->FaceCount * 3];

				for (DWORD idxIdx = 0; idxIdx < pRange9->FaceCount * 3; ++idxIdx)
				{
					uint16_t srcIdx = pIBData[pRange9->FaceStart * 3 + idxIdx];
					assert(srcIdx >= pRange9->VertexStart && srcIdx < pRange9->VertexStart + pRange9->VertexCount);

					pNewIBData[idxIdx] = uint16_t(srcIdx - pRange9->VertexStart);
				}

				pIndexBuffer = CreateImmutableIndexBuffer(pResult->m_pApp->GetDevice(), pRange9->FaceCount* 3 * sizeof(uint16_t), pNewIBData);

				pMeshIB9->Unlock();
				Release(pMeshIB9);

				delete[] pNewIBData;
			}

			// Looks good?
			if (pVertexBuffer && pIndexBuffer && pShader)
			{
				Subset *pSubset = &pResult->m_pSubsets[pResult->m_numSubsets++];

				pSubset->pShader = pShader;

				pSubset->firstItem = pRange9->FaceStart * 3;
				pSubset->numItems = pRange9->FaceCount * 3;

				pSubset->pVertexBuffer = pVertexBuffer;
				pVertexBuffer = NULL;

				pSubset->vtxStride = vtxStride11;

				pSubset->pIndexBuffer = pIndexBuffer;
				pIndexBuffer = NULL;

				pSubset->pTexture = pTexture;
				pTexture = NULL;

				pSubset->pTextureView = pTextureView;
				pTextureView = NULL;

				pSubset->pSamplerState = pSamplerState;
				pSamplerState = NULL;

				pSubset->localAABBMin = localAABBMin;
				pSubset->localAABBMax = localAABBMax;
			}

			Release(pVertexBuffer);
			Release(pIndexBuffer);
			Release(pTexture);
			Release(pTextureView);
			Release(pSamplerState);
		}
	}

	delete[] pRanges9;

	return pResult;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
