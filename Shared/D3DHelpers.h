#ifndef HEADER_2BB91D124FA14482B0F30C829823966A
#define HEADER_2BB91D124FA14482B0F30C829823966A

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include <stdint.h>

struct ID3D11ShaderReflection;
struct D3DXVECTOR2;
struct D3DXVECTOR3;
struct D3DXVECTOR4;
struct D3DXMATRIX;
struct _D3D_SHADER_MACRO;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct VertexColour
{
	uint8_t r,g,b,a;

	VertexColour();
	explicit VertexColour(uint32_t bgra);
	VertexColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Release D3D/COM object and set pointer to it to NULL.

template<class T>
static inline void Release(T *&p)
{
	if(p)
	{
		p->Release();
		p = 0;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void dprintf(const char *pFmt, ...);
void dputs(const char *str);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Get name for various D3D enums.

const char *GetNameD3D_FEATURE_LEVEL(int v);
const char *GetNameD3D_SHADER_VARIABLE_CLASS(int v);
const char *GetNameD3D_SHADER_VARIABLE_TYPE(int v);
const char *GetNameD3D_SHADER_INPUT_TYPE(int v);
const char *GetNameD3D_RESOURCE_RETURN_TYPE(int v);
const char *GetNameD3D_SRV_DIMENSION(int v);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Set the debug name of a D3D object (shader, texture, buffer, etc.).
// D3D doesn't use the name for anything, so it's entirely optional,
// and it can be whatever you like.
//
// The debug name is helpful for working out which specific
// objects D3D is referring to, if it tells you that the program is
// leaking objects when it quits. For example:
//
// D3D11: WARNING: Live Texture2D: Name="BackBuffer 1171x729", Addr=0x00435C2C, ExtRef=0, IntRef=1 [ STATE_CREATION WARNING #2097235: LIVE_TEXTURE2D ]

void SetD3DObjectDebugName(ID3D11DeviceChild *pD3DObject, const char *pFmt, ...);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// The sampler state is optional. This function hands out the same one
// one each time, so once you have one you can just use it for
// everything.
//
// *ppD3DTexture, *ppD3DTextureResourceView and *ppSamplerState are
// all Release'd before loading.
bool LoadTextureFromFile(
	ID3D11Device *pD3DDevice, const char *pFileName, 
	ID3D11Texture2D **ppD3DTexture, ID3D11ShaderResourceView **ppD3DTextureResourceView, ID3D11SamplerState **ppSamplerState);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Set shader profiles for the compiler.
//
// Most of the time, all the shaders will use the same profile.
// Rather than set it several times, set it once here.
//
// The contents of each string are copied.
//
// Default profiles are "vs_4_0" and "ps_4_0".

void SetShaderProfiles(const char *pVSProfile, const char *pPSProfile);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

extern const int INVALID_PACK_OFFSET;

class ShaderDescription
{
public:
	ShaderDescription();
	~ShaderDescription();

	bool SetFromShaderBlob(ID3D10Blob *pD3DShaderBlob);

	// All the FindXXX functions return false if not found, and
	// set the slot/pack offset to less than zero. This gives later
	// code the option of working around missing constants.
	//
	// Finding something is not guaranteed to be very efficient.
	// The idea is to find indexes of everything of interest when
	// the shader is first loaded.
	//
	// (Alternatively, specify slots and packoffsets manually in
	// the shader.)

	// Any globals that are outside a cbuffer get assigned to
	// a special cbuffer called "$Globals"; this is built-in
	// DirectX behaviour.
	bool FindCBuffer(const char *pName,int *pSlot) const;

	size_t GetCBufferSizeBytes(int cbufferSlot) const;

	bool FindFloat4x4(int cbufferSlot, const char *pName, int *pPackOffset) const;
	bool FindFloat4(int cbufferSlot, const char *pName, int *pPackOffset) const;
	bool FindFloat3(int cbufferSlot, const char *pName, int *pPackOffset) const;
	bool FindFloat2(int cbufferSlot, const char *pName, int *pPackOffset) const;
	bool FindFloat(int cbufferSlot, const char *pName, int *pPackOffset) const;
	bool FindInt(int cbufferSlot, const char *pName, int *pPackOffset) const;

	bool FindTexture(const char *pName, int *pSlot) const;
	bool FindSamplerState(const char *pName, int *pSlot) const;
protected:
private:
	struct ThingDescription;
	struct CBufferDescription;

	CBufferDescription *m_pCBuffers;
	ThingDescription *m_pTextures;
	ThingDescription *m_pSamplerStates;

	bool FindThing(const ThingDescription *pThings, const char *pName, int thingType, int *pSlot) const;
	bool FindCBufferThing(int cbufferSlot, const char *pName, int thingType, int *pPackOffset) const;
	void DeleteThings(ThingDescription *pThings);
	void Destruct();
	ThingDescription *GetResourceThingList(ID3D11ShaderReflection *pRShader, int d3dShaderInputType, int thingType);
	void DumpThingList(const ThingDescription *pThings, const char *pPrefix) const;

	ShaderDescription(const ShaderDescription &);
	ShaderDescription &operator=(const ShaderDescription &);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Set cbuffer variables. Map the cbuffer of interest, and pass in the
// pack offsets from the shader description. If setting an array, use
// the second form, that takes an array index as well.
//
// (Alternatively, specify pack offsets manually in the shader, and
// pass them in to these functions.)
//
// If the pack offset is <0, nothing happens. This is designed to
// interact neatly with the ShaderDescription class.
//
// SetFloat4x4 automatically transposes the matrix, so you don't have
// to.

void SetCBufferFloat4x4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXMATRIX &mtx);
void SetCBufferFloat4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR4 &vec);
void SetCBufferFloat3(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR3 &vec);
void SetCBufferFloat2(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, const D3DXVECTOR2 &vec);
void SetCBufferFloat(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, float f);
void SetCBufferInt(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int i);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Note that there is no bounds checking for arrays.

void SetCBufferArrayFloat4x4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXMATRIX &mtx);
void SetCBufferArrayFloat4(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR4 &vec);
void SetCBufferArrayFloat3(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR3 &vec);
void SetCBufferArrayFloat2(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, const D3DXVECTOR2 &vec);
void SetCBufferArrayFloat(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, float f);
void SetCBufferArrayInt(const D3D11_MAPPED_SUBRESOURCE &mapping, int packOffset, int arrayIndex, int i);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Compile VS and PS, assuming both come from the same file or string.
//
// pVSDescription and pPSDescription may be NULL, if you aren't
// interested.
//
// The shaders are compiled to expect column-major matrices.
// When using `SetCBufferFloat4x4' above, this is all taken care of.

bool CompileShadersFromFile(
	ID3D11Device *pD3DDevice, const char *pFileName,
	const char *pVSEntryPoint, ID3D11VertexShader **ppD3DVertexShader, ShaderDescription *pVSDescription,
	const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, unsigned numInputElementDescs, ID3D11InputLayout **ppD3DInputLayout,
	const char *pPSEntryPoint, ID3D11PixelShader **ppD3DPixelShader, ShaderDescription *pPSDescription,
	const _D3D_SHADER_MACRO *pMacros);

bool CompileShadersFromString(
	ID3D11Device *pD3DDevice, const char *pShaderSource,
	const char *pVSEntryPoint, ID3D11VertexShader **ppD3DVertexShader, ShaderDescription *pVSDescription,
	const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, unsigned numInputElementDescs, ID3D11InputLayout **ppD3DInputLayout,
	const char *pPSEntryPoint, ID3D11PixelShader **ppD3DPixelShader, ShaderDescription *pPSDescription,
	const _D3D_SHADER_MACRO *pMacros);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Buffer *CreateBuffer(ID3D11Device *pDevice, UINT sizeBytes, D3D11_USAGE usage, UINT bindFlags, UINT cpuAccessFlags, const void *pInitialData);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Create vertex and index buffers from a block of data.

ID3D11Buffer *CreateImmutableVertexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData);
ID3D11Buffer *CreateImmutableIndexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData);

ID3D11Buffer *CreateDynamicVertexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData);
ID3D11Buffer *CreateDynamicIndexBuffer(ID3D11Device *pDevice, UINT sizeBytes, const void *pInitialData);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#endif//HEADER_2BB91D124FA14482B0F30C829823966A
