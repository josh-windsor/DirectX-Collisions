#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <DxErr.h>

// this is for D3DX maths, which D3D11 has replaced with the more
// efficient but rather inconvenient-sounding XNA Maths.
#include <D3DX10math.h>

#include "D3DHelpers.h"

#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

VertexColour::VertexColour():
r(0),
g(0),
b(0),
a(0)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

VertexColour::VertexColour(uint32_t bgra):
r(uint8_t(bgra>>16)),
g(uint8_t(bgra>>8)),
b(uint8_t(bgra>>0)),
a(uint8_t(bgra>>24))
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

VertexColour::VertexColour(uint8_t rArg, uint8_t gArg, uint8_t bArg, uint8_t aArg):
r(rArg),
g(gArg),
b(bArg),
a(aArg)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// `dprintf' works like printf, but it prints to stdout and the
// debugger, so you have a record of the output even after the program
// is finished.
//
// Also makes it easy to get rid of all your debug prints.
//
// There is a built-in D3D thing (`DXTrace') that does something
// quite similar, but it's a pain to use without the format string.
void dprintf(const char *pFmt, ...)
{
	char buf[8192];
	va_list v;

	va_start(v, pFmt);

	_vsnprintf(buf, sizeof buf, pFmt, v);
	buf[sizeof buf - 1] = 0;

	va_end(v);

	OutputDebugString(buf);

	va_start(v, pFmt);

	vfprintf(stdout, pFmt, v);

	va_end(v);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Space for dprintf expansion is limited.
//
// For printing large buffers and/or when format strings aren't
// needed (e.g., D3D shader disassembly), this can be used.
void dputs(const char *str)
{
	OutputDebugString(str);

	fputs(str, stdout);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define BEGIN_NAMES(TYPE)\
	const char *GetName##TYPE(int v)\
	{\
		switch(v)\
		{\
		default:\
			return "?" #TYPE "?";

#define END_NAMES()\
		}\
	}

#define NAME(X) case (X): return #X;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_FEATURE_LEVEL)
	NAME(D3D_FEATURE_LEVEL_9_1)
	NAME(D3D_FEATURE_LEVEL_9_2)
	NAME(D3D_FEATURE_LEVEL_9_3)
	NAME(D3D_FEATURE_LEVEL_10_0)
	NAME(D3D_FEATURE_LEVEL_10_1)
	NAME(D3D_FEATURE_LEVEL_11_0)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_VARIABLE_CLASS)
	NAME(D3D_SVC_SCALAR)
	NAME(D3D_SVC_VECTOR)
	NAME(D3D_SVC_MATRIX_ROWS)
	NAME(D3D_SVC_MATRIX_COLUMNS)
	NAME(D3D_SVC_OBJECT)
	NAME(D3D_SVC_STRUCT)
	NAME(D3D_SVC_INTERFACE_CLASS)
	NAME(D3D_SVC_INTERFACE_POINTER)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_VARIABLE_TYPE)
    NAME(D3D_SVT_VOID)
	NAME(D3D_SVT_BOOL)
	NAME(D3D_SVT_INT)
	NAME(D3D_SVT_FLOAT)
	NAME(D3D_SVT_STRING)
	NAME(D3D_SVT_TEXTURE)
	NAME(D3D_SVT_TEXTURE1D)
	NAME(D3D_SVT_TEXTURE2D)
	NAME(D3D_SVT_TEXTURE3D)
	NAME(D3D_SVT_TEXTURECUBE)
	NAME(D3D_SVT_SAMPLER)
	NAME(D3D_SVT_SAMPLER1D)
	NAME(D3D_SVT_SAMPLER2D)
	NAME(D3D_SVT_SAMPLER3D)
	NAME(D3D_SVT_SAMPLERCUBE)
	NAME(D3D_SVT_PIXELSHADER)
	NAME(D3D_SVT_VERTEXSHADER)
	NAME(D3D_SVT_PIXELFRAGMENT)
	NAME(D3D_SVT_VERTEXFRAGMENT)
	NAME(D3D_SVT_UINT)
	NAME(D3D_SVT_UINT8)
	NAME(D3D_SVT_GEOMETRYSHADER)
	NAME(D3D_SVT_RASTERIZER)
	NAME(D3D_SVT_DEPTHSTENCIL)
	NAME(D3D_SVT_BLEND)
	NAME(D3D_SVT_BUFFER)
	NAME(D3D_SVT_CBUFFER)
	NAME(D3D_SVT_TBUFFER)
	NAME(D3D_SVT_TEXTURE1DARRAY)
	NAME(D3D_SVT_TEXTURE2DARRAY)
	NAME(D3D_SVT_RENDERTARGETVIEW)
	NAME(D3D_SVT_DEPTHSTENCILVIEW)
	NAME(D3D_SVT_TEXTURE2DMS)
	NAME(D3D_SVT_TEXTURE2DMSARRAY)
	NAME(D3D_SVT_TEXTURECUBEARRAY)
	NAME(D3D_SVT_HULLSHADER)
	NAME(D3D_SVT_DOMAINSHADER)
	NAME(D3D_SVT_INTERFACE_POINTER)
	NAME(D3D_SVT_COMPUTESHADER)
	NAME(D3D_SVT_DOUBLE)
	NAME(D3D_SVT_RWTEXTURE1D)
	NAME(D3D_SVT_RWTEXTURE1DARRAY)
	NAME(D3D_SVT_RWTEXTURE2D)
	NAME(D3D_SVT_RWTEXTURE2DARRAY)
	NAME(D3D_SVT_RWTEXTURE3D)
	NAME(D3D_SVT_RWBUFFER)
	NAME(D3D_SVT_BYTEADDRESS_BUFFER)
	NAME(D3D_SVT_RWBYTEADDRESS_BUFFER)
	NAME(D3D_SVT_STRUCTURED_BUFFER)
	NAME(D3D_SVT_RWSTRUCTURED_BUFFER)
	NAME(D3D_SVT_APPEND_STRUCTURED_BUFFER)
	NAME(D3D_SVT_CONSUME_STRUCTURED_BUFFER)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_INPUT_TYPE)
	NAME(D3D_SIT_CBUFFER)
	NAME(D3D_SIT_TBUFFER)
	NAME(D3D_SIT_TEXTURE)
	NAME(D3D_SIT_SAMPLER)
	NAME(D3D_SIT_UAV_RWTYPED)
	NAME(D3D_SIT_STRUCTURED)
	NAME(D3D_SIT_UAV_RWSTRUCTURED)
	NAME(D3D_SIT_BYTEADDRESS)
	NAME(D3D_SIT_UAV_RWBYTEADDRESS)
	NAME(D3D_SIT_UAV_APPEND_STRUCTURED)
	NAME(D3D_SIT_UAV_CONSUME_STRUCTURED)
	NAME(D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_RESOURCE_RETURN_TYPE)
	NAME(D3D_RETURN_TYPE_UNORM)
	NAME(D3D_RETURN_TYPE_SNORM)
	NAME(D3D_RETURN_TYPE_SINT)
	NAME(D3D_RETURN_TYPE_UINT)
	NAME(D3D_RETURN_TYPE_FLOAT)
	NAME(D3D_RETURN_TYPE_MIXED)
	NAME(D3D_RETURN_TYPE_DOUBLE)
	NAME(D3D_RETURN_TYPE_CONTINUED)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SRV_DIMENSION)
	NAME(D3D_SRV_DIMENSION_UNKNOWN)
	NAME(D3D_SRV_DIMENSION_BUFFER)
	NAME(D3D_SRV_DIMENSION_TEXTURE1D)
	NAME(D3D_SRV_DIMENSION_TEXTURE1DARRAY)
	NAME(D3D_SRV_DIMENSION_TEXTURE2D)
	NAME(D3D_SRV_DIMENSION_TEXTURE2DARRAY)
	NAME(D3D_SRV_DIMENSION_TEXTURE2DMS)
	NAME(D3D_SRV_DIMENSION_TEXTURE2DMSARRAY)
	NAME(D3D_SRV_DIMENSION_TEXTURE3D)
	NAME(D3D_SRV_DIMENSION_TEXTURECUBE)
	NAME(D3D_SRV_DIMENSION_TEXTURECUBEARRAY)
	NAME(D3D_SRV_DIMENSION_BUFFEREX)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void SetD3DObjectDebugName(ID3D11DeviceChild *pD3DObject, const char *pFmt, ...)
{
	va_list v;
	char buf[100];

	va_start(v, pFmt);

	_vsnprintf(buf, sizeof buf, pFmt, v);
	buf[sizeof buf - 1] = 0;

	va_end(v);

	pD3DObject->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool LoadTextureFromFile(
	ID3D11Device *pD3DDevice, 
	const char *pFileName, 
	ID3D11Texture2D **ppD3DTexture, 
	ID3D11ShaderResourceView **ppD3DTextureResourceView, 
	ID3D11SamplerState **ppD3DSamplerState)
{
	Release(*ppD3DTexture);
	Release(*ppD3DTextureResourceView);

	if (ppD3DSamplerState)
		Release(*ppD3DSamplerState);

	static const DXGI_FORMAT IMAGE_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Load texture.
	{
		D3DX11_IMAGE_LOAD_INFO imageLoadInfo;

		imageLoadInfo.Format = IMAGE_FORMAT;

		if (FAILED(D3DX11CreateTextureFromFile(pD3DDevice, pFileName, &imageLoadInfo, NULL, (ID3D11Resource **)ppD3DTexture, NULL)))
			goto bad;
	}

	// Create texture resource view.
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;

		resViewDesc.Format = IMAGE_FORMAT;
		resViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		resViewDesc.Texture2D.MostDetailedMip = 0;
		resViewDesc.Texture2D.MipLevels = ~0u;

		//ULONG a = m_pTexture->AddRef() - 1;
		//m_pTexture->Release();

		if (FAILED(pD3DDevice->CreateShaderResourceView(*ppD3DTexture, &resViewDesc, ppD3DTextureResourceView)))
			goto bad;

		//ULONG bA = m_pTexture->AddRef() - 1;
		//m_pTexture->Release();
	}

	// Create sampler state.
	if(ppD3DSamplerState)
	{
		D3D11_SAMPLER_DESC samplerDesc;

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.f;
		samplerDesc.BorderColor[1] = 0.f;
		samplerDesc.BorderColor[2] = 0.f;
		samplerDesc.BorderColor[3] = 0.f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		if (FAILED(pD3DDevice->CreateSamplerState(&samplerDesc, ppD3DSamplerState)))
			goto bad;
	}

	return true;

bad:
	Release(*ppD3DTexture);
	Release(*ppD3DTextureResourceView);

	if (ppD3DSamplerState)
		Release(*ppD3DSamplerState);

	return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static char g_aVSProfile[100] = "vs_4_0";
static char g_aPSProfile[100] = "ps_4_0";

void SetShaderProfiles(const char *pVSProfile, const char *pPSProfile)
{
	strncpy(g_aVSProfile, pVSProfile, sizeof g_aVSProfile);
	g_aVSProfile[sizeof g_aVSProfile - 1] = 0;

	strncpy(g_aPSProfile, pPSProfile, sizeof g_aPSProfile);
	g_aPSProfile[sizeof g_aPSProfile - 1] = 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

enum ThingType
{
	TT_NONE,
	TT_TEXTURE2D,
	TT_SAMPLER_STATE,
	TT_FLOAT,
	TT_FLOAT2,
	TT_FLOAT3,
	TT_FLOAT4,
	TT_FLOAT4x4,
	TT_INT,
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(ThingType)
	NAME(TT_NONE)
	NAME(TT_TEXTURE2D)
	NAME(TT_SAMPLER_STATE)
	NAME(TT_FLOAT)
	NAME(TT_FLOAT2)
	NAME(TT_FLOAT3)
	NAME(TT_FLOAT4)
	NAME(TT_FLOAT4x4)
	NAME(TT_INT)
END_NAMES()

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct ShaderDescription::ThingDescription
{
	// name, as per HLSL
	char *pName;

	// type of "thing"
	ThingType type;

	// slot/packoffset (according to type)
	int slot;

	ThingDescription *pNext;

	ThingDescription();
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ShaderDescription::ThingDescription::ThingDescription():
pName(NULL),
pNext(NULL)
{
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct ShaderDescription::CBufferDescription
{
	// name, as per HLSL.
	char *pName;

	// size of buffer, in bytes
	size_t sizeBytes;

	// Constants.
	ThingDescription *pConstants;

	CBufferDescription();
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ShaderDescription::CBufferDescription::CBufferDescription():
pName(NULL),
sizeBytes(0),
pConstants(NULL)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This value is fixed by D3D.
static const int NUM_CBUFFER_SLOTS = 16;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ShaderDescription::ShaderDescription():
m_pCBuffers(NULL),
m_pTextures(NULL),
m_pSamplerStates(NULL)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ShaderDescription::~ShaderDescription()
{
	this->Destruct();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::SetFromShaderBlob(ID3D10Blob *pD3DShaderBlob)
{
	this->Destruct();

	ID3D11ShaderReflection *pRShader;
	if (FAILED(D3DReflect(pD3DShaderBlob->GetBufferPointer(), pD3DShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&pRShader)))
		return false;

	D3D11_SHADER_DESC shaderDesc;
	pRShader->GetDesc(&shaderDesc);

	// Add variables from constant buffers.

	m_pCBuffers = new CBufferDescription[NUM_CBUFFER_SLOTS];

	for (UINT bufferIdx = 0; bufferIdx < shaderDesc.ConstantBuffers; ++bufferIdx)
	{
		ID3D11ShaderReflectionConstantBuffer *pRBuffer = pRShader->GetConstantBufferByIndex(bufferIdx);

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		if (FAILED(pRBuffer->GetDesc(&bufferDesc)))
			continue;

		D3D11_SHADER_INPUT_BIND_DESC bufferBindDesc;
		if (FAILED(pRShader->GetResourceBindingDescByName(bufferDesc.Name, &bufferBindDesc)))
			continue;//bit mystifying...

		if (bufferBindDesc.BindPoint >= NUM_CBUFFER_SLOTS)
		{
			// D3D supplied an invalid cbuffer slot - shouldn't happen!
			continue;
		}

		CBufferDescription *pCBuffer = &m_pCBuffers[bufferBindDesc.BindPoint];

		if (pCBuffer->pName)
		{
			// 2 cbuffers bound to the same slot - shouldn't happen!
			continue;
		}

		pCBuffer->pName = _strdup(bufferDesc.Name);
		pCBuffer->sizeBytes = bufferDesc.Size;

		ThingDescription **ppNextThing = &pCBuffer->pConstants;

		for (UINT varIdx = 0; varIdx < bufferDesc.Variables; ++varIdx)
		{
			ID3D11ShaderReflectionVariable *pRVar = pRBuffer->GetVariableByIndex(varIdx);
			if (!pRVar)
				continue;

			D3D11_SHADER_VARIABLE_DESC varDesc;
			if (FAILED(pRVar->GetDesc(&varDesc)))
				continue;

			ID3D11ShaderReflectionType *pRVarType = pRVar->GetType();
			if (!pRVarType)
				continue;

			D3D11_SHADER_TYPE_DESC varTypeDesc;
			if (FAILED(pRVarType->GetDesc(&varTypeDesc)))
				continue;

			ThingType thingType = TT_NONE;

			if (varTypeDesc.Type == D3D_SVT_FLOAT && varTypeDesc.Rows == 1 && varTypeDesc.Columns == 1)
				thingType = TT_FLOAT;
			else if (varTypeDesc.Type == D3D_SVT_FLOAT && varTypeDesc.Rows == 1 && varTypeDesc.Columns == 2)
				thingType = TT_FLOAT2;
			else if (varTypeDesc.Type == D3D_SVT_FLOAT && varTypeDesc.Rows == 1 && varTypeDesc.Columns == 3)
				thingType = TT_FLOAT3;
			else if (varTypeDesc.Type == D3D_SVT_FLOAT && varTypeDesc.Rows == 1 && varTypeDesc.Columns == 4)
				thingType = TT_FLOAT4;
			else if (varTypeDesc.Type == D3D_SVT_FLOAT && varTypeDesc.Rows == 4 && varTypeDesc.Columns == 4)
				thingType = TT_FLOAT4x4;
			else if (varTypeDesc.Type == D3D_SVT_INT && varTypeDesc.Rows == 1 && varTypeDesc.Columns == 1)
				thingType = TT_INT;

			if (thingType != TT_NONE)
			{
				ThingDescription *pNewThing = new ThingDescription;

				pNewThing->pName = _strdup(varDesc.Name);
				pNewThing->type = thingType;
				pNewThing->slot = varDesc.StartOffset;

				*ppNextThing = pNewThing;
				ppNextThing = &pNewThing->pNext;
			}
		}
	}

	// Add shader inputs.
	m_pTextures = this->GetResourceThingList(pRShader, D3D_SIT_TEXTURE, TT_TEXTURE2D);
	m_pSamplerStates = this->GetResourceThingList(pRShader, D3D_SIT_SAMPLER, TT_SAMPLER_STATE);

	Release(pRShader);

	{
		dprintf("---8<--- BEGIN DUMP\n");
		dprintf("CBuffers:\n");
		for(int i = 0; i < NUM_CBUFFER_SLOTS; ++i)
		{
			dprintf("%d: ", i);
			if (!m_pCBuffers[i].pName)
				dprintf("(empty)\n");
			else
			{
				dprintf("\"%s\", %u bytes\n", m_pCBuffers[i].pName, m_pCBuffers[i].sizeBytes);
				this->DumpThingList(m_pCBuffers[i].pConstants, "    ");
			}
		}

		dprintf("Textures:\n");
		this->DumpThingList(m_pTextures,"    ");

		dprintf("Sampler States:\n");
		this->DumpThingList(m_pSamplerStates,"    ");
		dprintf("---8<--- END DUMP\n");
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void ShaderDescription::DumpThingList(const ThingDescription *pThings, const char *pPrefix) const
{
	int i = 0;

	for (const ThingDescription *pThing = pThings; pThing; pThing = pThing->pNext, ++i)
		dprintf("%s%d. \"%s\", %s, slot=%d\n", pPrefix, i, pThing->pName, GetNameThingType(pThing->type), pThing->slot);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

size_t ShaderDescription::GetCBufferSizeBytes(int cbufferSlot) const
{
	if (cbufferSlot < 0 || cbufferSlot >= NUM_CBUFFER_SLOTS)
		return 0;

	const CBufferDescription *pCBuffer = &m_pCBuffers[cbufferSlot];

	if (!pCBuffer->pName)
		return 0;

	return pCBuffer->sizeBytes;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindCBuffer(const char *pName,int *pSlot) const
{
	for (int i = 0; i < NUM_CBUFFER_SLOTS; ++i)
	{
		if (m_pCBuffers[i].pName)
		{
			if (strcmp(m_pCBuffers[i].pName, pName) == 0)
			{
				*pSlot = i;
				return true;
			}
		}
	}

	*pSlot = -1;
	return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindFloat4x4(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_FLOAT4x4, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindFloat4(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_FLOAT4, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindFloat3(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_FLOAT3, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindFloat2(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_FLOAT2, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindFloat(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_FLOAT, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindInt(int cbufferSlot, const char *pName, int *pPackOffset) const
{
	return this->FindCBufferThing(cbufferSlot, pName, TT_INT, pPackOffset);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindTexture(const char *pName, int *pSlot) const
{
	return this->FindThing(m_pTextures, pName, TT_TEXTURE2D, pSlot);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindSamplerState(const char *pName, int *pSlot) const
{
	return this->FindThing(m_pSamplerStates, pName, TT_SAMPLER_STATE, pSlot);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindThing(const ThingDescription *pThings, const char *pName, int thingType, int *pSlot) const
{
	for (const ThingDescription *pThing = pThings; pThing; pThing = pThing->pNext)
	{
		if (strcmp(pThing->pName, pName) == 0)
		{
			if (pThing->type == thingType)
			{
				*pSlot = pThing->slot;
				return true;
			}
		}
	}

	*pSlot = -1;
	return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool ShaderDescription::FindCBufferThing(int cbufferSlot, const char *pName, int thingType, int *pPackOffset) const
{
	if (cbufferSlot < 0 || cbufferSlot >= NUM_CBUFFER_SLOTS)
	{
		*pPackOffset = -1;
		return false;
	}

	if (!this->FindThing(m_pCBuffers[cbufferSlot].pConstants, pName, thingType, pPackOffset))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void ShaderDescription::DeleteThings(ThingDescription *pThings)
{
	ThingDescription *pThing = pThings;

	while (pThing)
	{
		ThingDescription *pNext = pThing->pNext;

		free(pThing->pName);

		delete pThing;

		pThing = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void ShaderDescription::Destruct()
{
	if (m_pCBuffers)
	{
		for (int i = 0; i < NUM_CBUFFER_SLOTS; ++i)
			this->DeleteThings(m_pCBuffers[i].pConstants);
	}

	delete[] m_pCBuffers;
	m_pCBuffers = NULL;

	this->DeleteThings(m_pTextures);
	m_pTextures = NULL;

	this->DeleteThings(m_pSamplerStates);
	m_pSamplerStates = NULL;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ShaderDescription::ThingDescription *ShaderDescription::GetResourceThingList(ID3D11ShaderReflection *pRShader, int d3dShaderInputType, int thingType)
{
	D3D11_SHADER_DESC shaderDesc;
	if (FAILED(pRShader->GetDesc(&shaderDesc)))
		return NULL;

	ThingDescription *pFirstThing = NULL, **ppNextThing = &pFirstThing;

	for (UINT resIdx = 0; resIdx < shaderDesc.BoundResources; ++resIdx)
	{
		D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
		if (FAILED(pRShader->GetResourceBindingDesc(resIdx, &inputBindDesc)))
			continue;

		if (inputBindDesc.Type == d3dShaderInputType)
		{
			ThingDescription *pNewThing = new ThingDescription;

			pNewThing->pName = _strdup(inputBindDesc.Name);
			pNewThing->type = ThingType(thingType);
			pNewThing->slot = inputBindDesc.BindPoint;

			*ppNextThing = pNewThing;
			ppNextThing = &pNewThing->pNext;
		}
	}

	return pFirstThing;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void SetCBufferFloat4x4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXMATRIX &mtx)
{
	if (mapping.pData && packOffset >= 0)
	{
		D3DXMATRIX *pDest = reinterpret_cast<D3DXMATRIX *>(static_cast<char *>(mapping.pData) + packOffset);

		D3DXMatrixTranspose(pDest, &mtx);
	}
}

void SetCBufferFloat4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR4 &vec)
{
	if (mapping.pData && packOffset >= 0)
	{
		D3DXVECTOR4 *pDest = reinterpret_cast<D3DXVECTOR4 *>(static_cast<char *>(mapping.pData) + packOffset);

		*pDest = vec;
	}
}

void SetCBufferFloat3(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR3 &vec)
{
	if (mapping.pData && packOffset >= 0)
	{
		D3DXVECTOR3 *pDest = reinterpret_cast<D3DXVECTOR3 *>(static_cast<char *>(mapping.pData) + packOffset);

		*pDest = vec;
	}
}

void SetCBufferFloat2(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR2 &vec)
{
	if (mapping.pData && packOffset >= 0)
	{
		D3DXVECTOR2 *pDest = reinterpret_cast<D3DXVECTOR2 *>(static_cast<char *>(mapping.pData) + packOffset);

		*pDest = vec;
	}
}

void SetCBufferFloat(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, float f)
{
	if (mapping.pData && packOffset >= 0)
	{
		float *pDest = reinterpret_cast<float *>(static_cast<char *>(mapping.pData) + packOffset);

		*pDest = f;
	}
}

void SetCBufferInt(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int i)
{
	if (mapping.pData && packOffset >= 0)
	{
		int *pDest = reinterpret_cast<int *>(static_cast<char *>(mapping.pData) + packOffset);

		*pDest = i;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void SetCBufferArrayFloat4x4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXMATRIX &mtx)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		D3DXMATRIX *pDest = reinterpret_cast<D3DXMATRIX *>(static_cast<char *>(mapping.pData) + packOffset) + arrayIndex;

		D3DXMatrixTranspose(pDest, &mtx);
	}
}

void SetCBufferArrayFloat4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR4 &vec)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		D3DXVECTOR4 *pDest = reinterpret_cast<D3DXVECTOR4 *>(static_cast<char *>(mapping.pData) + packOffset) + arrayIndex;

		*pDest = vec;
	}
}

void SetCBufferArrayFloat3(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR3 &vec)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		// NOTE packing rules for HLSL constants: http://msdn.microsoft.com/en-us/library/bb509632(v=vs.85).aspx
		//
		// Executive summary: "Arrays are not packed in HLSL by default ...
		// every element in an array is stored in a four-component vector".
		D3DXVECTOR3 *pDest = reinterpret_cast<D3DXVECTOR3 *>(static_cast<char *>(mapping.pData) + packOffset + arrayIndex * 4 * sizeof(float));

		*pDest = vec;
	}
}

void SetCBufferArrayFloat2(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR2 &vec)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		// NOTE packing rules for HLSL constants: http://msdn.microsoft.com/en-us/library/bb509632(v=vs.85).aspx
		//
		// Executive summary: "Arrays are not packed in HLSL by default ...
		// every element in an array is stored in a four-component vector".
		D3DXVECTOR2 *pDest = reinterpret_cast<D3DXVECTOR2 *>(static_cast<char *>(mapping.pData) + packOffset + arrayIndex * 4 * sizeof(float));

		*pDest = vec;
	}
}

void SetCBufferArrayFloat(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, float f)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		// NOTE packing rules for HLSL constants: http://msdn.microsoft.com/en-us/library/bb509632(v=vs.85).aspx
		//
		// Executive summary: "Arrays are not packed in HLSL by default ...
		// every element in an array is stored in a four-component vector".
		float *pDest = reinterpret_cast<float *>(static_cast<char *>(mapping.pData) + packOffset + arrayIndex * 4 * sizeof(float));

		*pDest = f;
	}
}

void SetCBufferArrayInt(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, int i)
{
	if (mapping.pData && packOffset >= 0 && arrayIndex >= 0)
	{
		// NOTE packing rules for HLSL constants: http://msdn.microsoft.com/en-us/library/bb509632(v=vs.85).aspx
		//
		// Executive summary: "Arrays are not packed in HLSL by default ...
		// every element in an array is stored in a four-component vector".
		int *pDest = reinterpret_cast<int *>(static_cast<char *>(mapping.pData) + packOffset + arrayIndex * 4 * sizeof(float));

		*pDest = i;
	}
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static ID3D10Blob *CompileShader(const char *pFileName, const char *pShaderSource, const D3D_SHADER_MACRO *pMacros, const char *pEntryPoint, const char *pProfileName, ShaderDescription *pDescription)
{
	HRESULT hr;

	DWORD flags = 0;

	flags |= D3D10_SHADER_DEBUG;//include debug info
	flags |= D3D10_SHADER_SKIP_OPTIMIZATION;//don't optimize
	flags |= D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR;//more efficient matrix layout

	ID3D10Blob *pD3DShaderBlob, *pErrorsBlob;

	if (pFileName)
		hr = D3DX11CompileFromFile(pFileName, pMacros, NULL, pEntryPoint, pProfileName, flags, 0, NULL, &pD3DShaderBlob, &pErrorsBlob, NULL);
	else
		hr = D3DX11CompileFromMemory(pShaderSource, strlen(pShaderSource), NULL, pMacros, NULL, pEntryPoint, pProfileName, flags, 0, NULL, &pD3DShaderBlob, &pErrorsBlob, NULL);

	if (FAILED(hr))
	{
		// FAIL.
		//
		// Errors are worth printing with OutputDebugString as well
		// as printf. You can double-click them in the output window
		// and be taken to the offending line immediately.

		const char *pErrors = static_cast<const char *>(pErrorsBlob->GetBufferPointer());

		//size_t errorsSize = pErrorsBlob->GetBufferSize();
		//TASSERT(errorsSize == 0 || pErrors[errorsSize - 1] == 0);

		dprintf("\"%s\": %s\n", pFileName, DXGetErrorDescription(hr), DXGetErrorString(hr));
		dprintf("---8<--- Error output begins:\n");
		dprintf("Entry Point: \"%s\"\n", pEntryPoint);
		dprintf("Profile Name: \"%s\"\n", pProfileName);
		dprintf("%s", pErrors);
		dprintf("---8<--- Error output ends.\n");

		Release(pD3DShaderBlob);//will probably be NULL anyway though.
	}
	else
	{
		ID3D10Blob *pDisassembly;
		hr = D3DDisassemble(pD3DShaderBlob->GetBufferPointer(), pD3DShaderBlob->GetBufferSize(),
			D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING, NULL, &pDisassembly);
		if (FAILED(hr))
		{
			dprintf("Shader disassembly failed.\n");
		}
		else
		{
			dputs(static_cast<const char *>(pDisassembly->GetBufferPointer()));
			Release(pDisassembly);
		}

		if (pDescription)
			pDescription->SetFromShaderBlob(pD3DShaderBlob);
	}

	Release(pErrorsBlob);

	return pD3DShaderBlob;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CompileShaders(
	ID3D11Device *pD3DDevice, const char *pFileName, const char *pShaderSource,
	const char *pVSEntryPoint, ID3D11VertexShader **ppD3DVertexShader, ShaderDescription *pVSDescription,
	const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, unsigned numInputElementDescs, ID3D11InputLayout **ppD3DInputLayout,
	const char *pPSEntryPoint, ID3D11PixelShader **ppD3DPixelShader, ShaderDescription *pPSDescription,
	const D3D_SHADER_MACRO *pMacros)
{
	Release(*ppD3DVertexShader);
	Release(*ppD3DInputLayout);
	Release(*ppD3DPixelShader);

	ID3D10Blob *pBlob;

	// VS & input layout
	pBlob = CompileShader(pFileName, pShaderSource, pMacros, pVSEntryPoint, g_aVSProfile, pVSDescription);
	if (!pBlob)
		goto bad;

	if (FAILED(pD3DDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppD3DVertexShader)))
		goto bad;

	if (ppD3DInputLayout)
	{
		if (FAILED(pD3DDevice->CreateInputLayout(pInputElementDescs, numInputElementDescs, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), ppD3DInputLayout)))
			goto bad;
	}

	Release(pBlob);

	// PS
	pBlob = CompileShader(pFileName, pShaderSource, pMacros, pPSEntryPoint, g_aPSProfile, pPSDescription);
	if (!pBlob)
		goto bad;

	if (FAILED(pD3DDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppD3DPixelShader)))
		goto bad;

	Release(pBlob);

	// That seemed to work.
	SetD3DObjectDebugName(*ppD3DVertexShader, "%s:%s", pFileName, pVSEntryPoint);

	SetD3DObjectDebugName(*ppD3DPixelShader, "%s:%s", pFileName, pPSEntryPoint);

	return true;
	
bad:
	// Some kind of error...
	Release(pBlob);
	Release(*ppD3DVertexShader);
	Release(*ppD3DInputLayout);
	Release(*ppD3DPixelShader);

	return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CompileShadersFromFile(
	ID3D11Device *pD3DDevice, const char *pFileName,
	const char *pVSEntryPoint, ID3D11VertexShader **ppD3DVertexShader, ShaderDescription *pVSDescription,
	const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, unsigned numInputElementDescs, ID3D11InputLayout **ppD3DInputLayout,
	const char *pPSEntryPoint, ID3D11PixelShader **ppD3DPixelShader, ShaderDescription *pPSDescription,
	const D3D_SHADER_MACRO *pMacros)
{
	return CompileShaders(pD3DDevice, pFileName, NULL, 
		pVSEntryPoint, ppD3DVertexShader, pVSDescription,
		pInputElementDescs, numInputElementDescs, ppD3DInputLayout,
		pPSEntryPoint, ppD3DPixelShader, pPSDescription,
		pMacros);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CompileShadersFromString(
	ID3D11Device *pD3DDevice, const char *pShaderSource,
	const char *pVSEntryPoint, ID3D11VertexShader **ppD3DVertexShader, ShaderDescription *pVSDescription,
	const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, unsigned numInputElementDescs, ID3D11InputLayout **ppD3DInputLayout,
	const char *pPSEntryPoint, ID3D11PixelShader **ppD3DPixelShader, ShaderDescription *pPSDescription,
	const D3D_SHADER_MACRO *pMacros)
{
	return CompileShaders(pD3DDevice, NULL, pShaderSource,
		pVSEntryPoint, ppD3DVertexShader, pVSDescription,
		pInputElementDescs, numInputElementDescs, ppD3DInputLayout,
		pPSEntryPoint, ppD3DPixelShader, pPSDescription,
		pMacros);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateBuffer(ID3D11Device *pDevice, UINT sizeBytes, D3D11_USAGE usage, UINT bindFlags, UINT cpuAccessFlags, const void *pInitialData)
{
	if (sizeBytes == 0)
		return NULL;

	D3D11_BUFFER_DESC desc;

	desc.ByteWidth = sizeBytes;
	desc.Usage = usage;
	desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = cpuAccessFlags;
	desc.MiscFlags = 0;

	// "Structure" here is a D3D notion. The value is correctly
	// zero.
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;

	data.pSysMem = pInitialData;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	ID3D11Buffer *pBuffer;
	if (FAILED(pDevice->CreateBuffer(&desc, pInitialData ? &data : NULL, &pBuffer)))
		return NULL;

	return pBuffer;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateImmutableVertexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData)
{
	return CreateBuffer(pDevice, sizeBytes, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, pInitialData);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateImmutableIndexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData)
{
	return CreateBuffer(pDevice, sizeBytes, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, pInitialData);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateDynamicVertexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData)
{
	return CreateBuffer(pDevice, sizeBytes, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, pInitialData);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateDynamicIndexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData)
{
	return CreateBuffer(pDevice, sizeBytes, D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE, pInitialData);
}

