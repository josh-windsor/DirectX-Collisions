#ifndef HEADER_7D3DEAFDCF424316BD4E7E77D930F670
#define HEADER_7D3DEAFDCF424316BD4E7E77D930F670

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include "App.h"
#include "D3DHelpers.h"
#include <D3DX10math.h>

#include <DirectXMath.h>
using namespace DirectX;

//struct ID3D11BlendState;
//struct ID3D11DepthStencilState;
//struct ID3D11RasterizerState;
//struct D3D11_INPUT_ELEMENT_DESC;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// The CommonApp class provides some standard simplified stuff
// shared between various apps.
//
// There's nothing in it that you couldn't do yourself, and it doesn't
// expose the full flexibility of D3D, but it is less verbose
// than doing everything by hand.
//
// Only requirements are:-
//
// 1. If you have a HandleStart function, call the CommonApp
//    one at the start of it, and returning false if it returns
//    false:
//
//        if (!this->CommonApp::HandleStart())
//            return false;
//
// 2. If you have a HandleStop function (you probably will, if you
//    have a HandleStart...), call the CommonApp equivalent at the end:
//
//        this->CommondApp::HandleStop();
//
//    HandleStop is a void function.
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Standard vertex types.

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This is the vertex type to use with the DrawUntextured function.

struct Vertex_Pos3fColour4ub
{
	D3DXVECTOR3	pos;
	VertexColour colour;

	Vertex_Pos3fColour4ub();
	Vertex_Pos3fColour4ub(const XMFLOAT3 &pos, VertexColour colour);
	Vertex_Pos3fColour4ub(const XMVECTOR &pos, VertexColour colour);
};

extern const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ub[];
extern const unsigned g_vertexDescSize_Pos3fColour4ub;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This is the vertex type to use with the DrawUntexturedLit function.

struct Vertex_Pos3fColour4ubNormal3f
{
	D3DXVECTOR3	pos;
	VertexColour colour;
	D3DXVECTOR3 normal;

	Vertex_Pos3fColour4ubNormal3f();
	Vertex_Pos3fColour4ubNormal3f(const XMFLOAT3 &pos, VertexColour colour, const XMFLOAT3 &normal);
	Vertex_Pos3fColour4ubNormal3f(const XMVECTOR &pos, VertexColour colour, const XMVECTOR &normal);
};

extern const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubNormal3f[];
extern const unsigned g_vertexDescSize_Pos3fColour4ubNormal3f;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This is the vertex type to use with the DrawTextured function.

struct Vertex_Pos3fColour4ubTex2f
{
	D3DXVECTOR3 pos;
	VertexColour colour;
	D3DXVECTOR2 tex;

	Vertex_Pos3fColour4ubTex2f();
	Vertex_Pos3fColour4ubTex2f(const XMFLOAT3 &pos, VertexColour colour, const XMFLOAT2 &tex);
	Vertex_Pos3fColour4ubTex2f(const XMVECTOR &pos, VertexColour colour, const XMVECTOR &tex);
};

extern const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubTex2f[];
extern const unsigned g_vertexDescSize_Pos3fColour4ubTex2f;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct Vertex_Pos3fColour4ubNormal3fTex2f
{
	D3DXVECTOR3 pos;
	VertexColour colour;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex;

	Vertex_Pos3fColour4ubNormal3fTex2f();
	Vertex_Pos3fColour4ubNormal3fTex2f(const XMFLOAT3 &pos, VertexColour colour, const XMFLOAT3 &normal, const XMFLOAT2 &tex);
	Vertex_Pos3fColour4ubNormal3fTex2f(const XMVECTOR &pos, VertexColour colour, const XMVECTOR &normal, const XMFLOAT2 &tex);
	Vertex_Pos3fColour4ubNormal3fTex2f(const XMVECTOR &pos, VertexColour colour, const XMVECTOR &normal, const XMVECTOR &tex);
};

extern const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubNormal3fTex2f[];
extern const unsigned g_vertexDescSize_Pos3fColour4ubNormal3fTex2f;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class CommonApp:
public App
{
public:
	CommonApp();
	~CommonApp();

	ID3D11Device *GetDevice() const;
	ID3D11DeviceContext *GetDeviceContext() const;

	float GetWindowAspectRatio() const;

	// Actually, the width and height will always be integer values. But
	// you usually end up needing them as floats anyway.
	void GetWindowSize(float *pWidth, float *pHeight) const;

	// Set world, view and project matrices.
	void SetWorldMatrix(const D3DXMATRIX &worldMtx);
	void SetWorldMatrix(const XMMATRIX &worldMtx);
	void SetViewMatrix(const XMMATRIX &viewMtx);
	void SetViewMatrix(const D3DXMATRIX &viewMtx);
	void SetProjectionMatrix(const XMMATRIX &projectionMtx);
	void SetProjectionMatrix(const D3DXMATRIX &projectionMtx);

	// Simplified camera setup. FOV is PI/4.
	void SetDefaultProjectionMatrix(float aspect = 1.f);

	// Simplied viewer setup.
	void SetDefaultViewMatrix(const D3DXVECTOR3 &camera, const D3DXVECTOR3 &lookat, const D3DXVECTOR3 &up);

	// Draw using one of the supplied shaders.
	//
	// Function        Texture   Lighting   Vertex Type                   Pixel Colour
	// --------      | ------- | -------- | -----------                 | ------------
	// Untextured        NO        NO       Pos3fColour4ub                Constant Colour * Vertex Colour
	// UntexturedLit     NO        YES      Pos3fColour4ubNormal3f        Lighting * Constant Colour * Vertex Colour
	// Textured          YES       NO       Pos3fColour4ubTex2f           Constant Colour * Vertex Colour * Texture
	// TexturedLit       YES       YES      Pos3fColour4ubNormal3fTex2f   Lighting * Constant Colour * Vertex Colour * Texture
	// 
	// where:
	//
	// `Constant Colour' (RGBA) is set by the SetConstantColour function.
	// 
	// `Lighting' (RGB) is the sum of the effect of all the lights. (The alpha value is assumed to be 1.0.)
	//
	// `Vertex Colour' (RGBA) is according to each vertex's `colour' member.
	//
	// `Texture' (RGBA) is the texel from the supplied texture according to each vertex's `tex' member.

	void DrawUntextured(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems);
	void DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems);
	void DrawTextured(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler);
	void DrawTexturedLit(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler);

	// Draw using a particular shader.
	//
	// The framework tries to set up certain constants in a cbuffer called
	// CommonApp, as follows.
	//
	// Missing constants are ignored. Constants not listed here are not
	// changed.
	//
	// Type           Name                    Value
	// ----         | ----                  | -----
	// float4x4       g_WVP                   World * View * Projection
	// float4x4       g_InvXposeW             Transpose(Inverse(World))
	// float4x4       g_W                     World
	// float4         g_constantColour        Constant Colour
	// float4         g_lightDirections[]     .xyz = Light World Direction
	//                                        .w = 1.0 (0.0 for point lights)
	// float4         g_lightPositions[]      .xyz = Light World Position
	//                                        .w = 1.0 (0.0 for directional lights)
	// float3         g_lightColours[]        Light Colour
	// float4         g_lightAttenuations[]   Light Attenuation values
	//                                        .x = a0
	//                                        .y = a1
	//                                        .z = a2
	//                                        .w = range * range
	// float4         g_lightSpots[]          Spot Light Factors
	//                                        .x = cos(theta/2)
	//                                        .y = cos(phi/2)
	//                                        .z = cos(theta/2) - cos(phi/2)
	//                                        .w = falloff
	// int            g_numLights             Number of enabled lights
	// Texture2D      g_texture               Texture, as per pTextureView
	// SamplerState   g_sampler               Sampler state, as per pTextureSampler
	//
	// The various light arrays are assumed to be of dimension
	// MAX_NUM_LIGHTS. They are filled in contiguously, even if the
	// enabled lights aren't contiguous.
	//
	class Shader;
	void DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, size_t vertexStride, ID3D11Buffer *pIndexBuffer, unsigned firstItem, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler, Shader *pShader);

	// Set constant colour.
	void SetConstantColour(const D3DXVECTOR4 &constantColour);

	//
	static const int MAX_NUM_LIGHTS = 4;
	void DisableLight(int light);
	void EnableDirectionalLight(int light, const XMFLOAT3 &worldDirection, const XMFLOAT3 &diffuseColour);
	void EnablePointLight(int light, const XMFLOAT3 &worldPosition, const XMFLOAT3 &diffuseColour);
	void EnableSpotLight(int light, const XMFLOAT3 &worldPosition, const XMFLOAT3 &worldDirection, float theta, float phi, float falloff, const XMFLOAT3 &diffuseColour);
	void SetLightAttenuation(int light, float range, float a0, float a1, float a2);

	// Set up blend state.
	static const bool DEFAULT_BLEND_ENABLE = true;
	void SetBlendState(bool blendEnable);

	// Set up depth/stencil state.
	static const bool DEFAULT_DEPTH_TEST = true;
	static const bool DEFAULT_DEPTH_WRITE = true;
	void SetDepthStencilState(bool depthTest, bool depthWrite = DEFAULT_DEPTH_WRITE);

	// Set up rasterizer state.
	static const bool DEFAULT_BACK_FACE_CULL = false;
	static const bool DEFAULT_WIREFRAME = false;
	void SetRasterizerState(bool backFaceCull, bool wireframe = DEFAULT_WIREFRAME);

	// Get sampler state, for use with the Draw functions.
	static const bool DEFAULT_BILINEAR = false;
	static const bool DEFAULT_MIPMAP = false;
	static const bool DEFAULT_WRAP = false;
	ID3D11SamplerState *GetSamplerState(bool bilinear = DEFAULT_BILINEAR, bool mipmap = DEFAULT_MIPMAP, bool wrap = DEFAULT_WRAP);

	// Clear default render target.
	void Clear(const XMFLOAT4 &clearColour);

	// Poll key state.
	bool IsKeyPressed(int vkey) const;

	// The `Shader' struct describes a shader. It has pointers to the
	// relevant D3D shader objects, and a list of ints, holding the
	// pack offsets/slots (as appropriate) of each shader input value
	// that the rendering knows how to supply.
	//
	// Before drawing something, the code selects the shader, and then
	// uses the pack offsets/slots to fill in the values it is
	// passing to the shader. If the shader doesn't require a particular
	// value, the pack offset/slot is -1, so the code knows not to
	// supply it.
	//
	// This shader struct is specific to the examples, and is an example
	// of a particular approach rather than something general-purpose.

	struct ShaderVars
	{
		int cbuffer;

		int wvp;
		int invXposeW;
		int w;
		int constantColour;
		int lightDirections;
		int lightPositions;
		int lightColours;
		int lightAttenuations;
		int lightSpots;
		int numLights;

		ShaderVars();
	};

	class Shader
	{
	public:
		ShaderVars vsGlobals, psGlobals;
		//// Pack offset or slot (as appropriate) of each shader input.
		//// Any shader
		//int vsGlobals;
		//int vsWVP;
		//int vsInvXposeW;
		//int vsW;
		//int vsConstantColour;
		//int vsLightDirections;
		//int vsLightPositions;
		//int vsLightColours;
		//int vsLightAttenuations;
		//int vsLightSpots;
		//int vsNumLights;

		int psTexture;
		int psSampler;

		ID3D11VertexShader *pVS;
		ID3D11PixelShader *pPS;
		ID3D11InputLayout *pIL;

		ID3D11Buffer *pVSCBuffer;
		ID3D11Buffer *pPSCBuffer;

		Shader();
		~Shader();

		void Reset();
	protected:
	private:
		Shader(const Shader &);
		Shader &operator=(const Shader &);
	};

	// Compile shader code from a string or a file, and fill out a Shader object for it.
	bool CompileShaderFromString(Shader *pShader, const char *pShaderCode, const D3D_SHADER_MACRO *pMacros, const D3D11_INPUT_ELEMENT_DESC *pInputElementsDescs, unsigned numInputElementsDescs);
	bool CompileShaderFromFile(Shader *pShader, const char *pShaderFileName, const D3D_SHADER_MACRO *pMacros, const D3D11_INPUT_ELEMENT_DESC *pInputElementsDescs, unsigned numInputElementsDescs);

	// Suitable if you've used CompileShadersFromFile or CompileShadersFromString.
	//
	// The Shader takes ownership of the D3D objects, and will Release them itself.
	void CreateShaderFromCompiledShader(Shader *pShader, ID3D11VertexShader *pVS, const ShaderDescription *pVSDescription, ID3D11InputLayout *pIL, ID3D11PixelShader *pPS, const ShaderDescription *pPSDescription); 

	Shader *GetUntexturedShader();
	Shader *GetUntexturedLitShader();
	Shader *GetTexturedShader();
	Shader *GetTexturedLitShader();
protected:
	bool HandleStart();
	void HandleStop();
private:
	struct Light
	{
		enum Type
		{
			Type_None,
			Type_Directional,
			Type_Point,
			Type_Spot,
		};

		Type type;
		D3DXVECTOR3 position;
		D3DXVECTOR3 direction;
		D3DXVECTOR3 diffuseColour;

		// Attenuation parameters.
		float a0, a1, a2, rangeSquared;

		// Spotlight parameters.
		float cosHalfTheta, cosHalfPhi, falloff;

		Light();
	};

	Light m_lights[MAX_NUM_LIGHTS];

	// D3D11 rather expects you to set up all the render state combinations
	// ahead of time. These arrays contain objects for each combination of
	// bools passed into the SetXXXState functions above, with the index
	// into the array calculated from the bool values.

	// TODO would it be better to expose the indexes directly??

	static const int BLEND_STATE_BLEND_ENABLE = 1 << 0;
	static const int NUM_BLEND_STATES = 2;
	ID3D11BlendState *m_apBlendStates[NUM_BLEND_STATES];

	static const int DEPTH_STENCIL_STATE_DEPTH_ENABLE = 1 << 0;
	static const int DEPTH_STENCIL_STATE_DEPTH_WRITE_ENABLE = 1 << 1;
	static const int NUM_DEPTH_STENCIL_STATES = 4;
	ID3D11DepthStencilState *m_apDepthStencilStates[NUM_DEPTH_STENCIL_STATES];

	static const int RASTERIZER_STATE_WIREFRAME = 1 << 0;
	static const int RASTERIZER_STATE_BACK_FACE_CULL = 1 << 1;
	static const int NUM_RASTERIZER_STATES = 4;
	ID3D11RasterizerState *m_apRasterizerStates[NUM_RASTERIZER_STATES];

	static const int SAMPLER_STATE_BILINEAR = 1 << 0;
	static const int SAMPLER_STATE_MIPMAP = 1 << 1;
	static const int SAMPLER_STATE_WRAP = 1 << 2;
	static const int NUM_SAMPLER_STATES = 8;
	ID3D11SamplerState *m_apSamplerStates[NUM_SAMPLER_STATES];

	Shader m_shaderUntextured;
	Shader m_shaderUntexturedLit;
	Shader m_shaderTextured;
	Shader m_shaderTexturedLit;

	// Current settings
	D3DXMATRIX m_projectionMtx;
	D3DXMATRIX m_viewMtx;
	D3DXMATRIX m_worldMtx;
	D3DXVECTOR4 m_constantColour;

	void GetWVP(D3DXMATRIX *pWVP) const;
	Light *GetLight(int light);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif//HEADER_7D3DEAFDCF424316BD4E7E77D930F670
