#include <d3d11.h>
#include <d3dx11.h>

#include "CommonApp.h"
#include "D3DHelpers.h"

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include <DirectXMath.h>
using namespace DirectX;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// It's more common to have the shaders come from a file. Much less
// annoying to edit, and much easier to read.
//
// This code stores them in a string so that the HLSL files don't
// have to be copied into each app's folder.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const char g_aShader[]=
	"cbuffer CommonApp\n"
	"{\n"
	"    float4x4 g_WVP;\n"
	"    float4 g_constantColour;\n"
	"\n"
	"#ifdef LIT\n"
	"    float4x4 g_InvXposeW;\n"
	"    float4x4 g_W;\n"
	"    float4 g_lightDirections[MAX_NUM_LIGHTS];\n"//(x,y,z,has-direction flag)
	"    float4 g_lightPositions[MAX_NUM_LIGHTS];\n"//(x,y,z,has-position flag)
	"    float3 g_lightColours[MAX_NUM_LIGHTS];\n"
	"    float4 g_lightAttenuations[MAX_NUM_LIGHTS];\n"//(a0,a1,a2,range^2)
	"    float4 g_lightSpots[MAX_NUM_LIGHTS];\n"//(cos(phi/2),cos(theta/2),1/(cos(theta/2)-cos(phi/2)),falloff)
	"    int g_numLights;\n"
	"#endif//LIT\n"
	"};\n"
	"\n"
	"#ifdef TEXTURED\n"
	"Texture2D g_texture;\n"
	"SamplerState g_sampler;\n"
	"#endif//TEXTURED\n"
	"\n"
	"struct VSInput\n"
	"{\n"
	"    float4 pos:POSITION;\n"
	"    float4 colour:COLOUR0;\n"
	"#ifdef LIT\n"
	"    float3 normal:NORMAL;\n"
	"#endif//LIT\n"
	"#ifdef TEXTURED\n"
	"    float2 tex:TEXCOORD;\n"
	"#endif//TEXTURED\n"
	"};\n"
	"\n"
	"struct PSInput\n"
	"{\n"
	"    float4 pos:SV_Position;\n"
	"    float4 colour:COLOUR0;\n"
	"#ifdef TEXTURED\n"
	"    float2 tex:TEXCOORD;\n"
	"#endif//TEXTURED\n"
	"};\n"
	"\n"
	"struct PSOutput\n"
	"{\n"
	"    float4 colour:SV_Target;\n"
	"};\n"
	"\n"
	"#ifdef LIT\n"
	"float4 GetLightingColour(float3 worldPos, float3 N)\n"
	"{\n"
	"\n"
	"    float4 lightingColour = float4(0, 0, 0, 1);\n"
	"\n"
	"    for (int i = 0; i < g_numLights; ++i)\n"
	"    {\n"
	"        float3 D = g_lightPositions[i].w * (g_lightPositions[i].xyz - worldPos);\n"
	"        float dotDD = dot(D, D);\n"
	"\n"
	"        if (dotDD > g_lightAttenuations[i].w)\n"
	"            continue;\n"
	"\n"
	"        float atten = 1.0 / (g_lightAttenuations[i].x + g_lightAttenuations[i].y * length(D) + g_lightAttenuations[i].z * dot(D, D));\n"
	"\n"
	"        float3 L = g_lightDirections[i].xyz;\n"
	"        float dotNL = g_lightDirections[i].w * saturate(dot(N, L));\n"
	"\n"
	"        float rho = 0.0;\n"
	"        if (dotDD > 0.0)\n"
	"            rho = dot(L, normalize(D));\n"//rho will be zero for point lights
	"\n"
	"        float spot;\n"
	"        if (rho > g_lightSpots[i].y)\n"
	"            spot = 1.0;\n"
	"        else if(rho < g_lightSpots[i].x)\n"
	"            spot = 0.0;\n"
	"        else\n"
	"            spot = pow((rho - g_lightSpots[i].x) * g_lightSpots[i].z, g_lightSpots[i].w);\n"
	"\n"
	"        float3 light = atten * spot * g_lightColours[i];\n"
	"        if (g_lightDirections[i].w > 0.f)\n"
	"            light *= dotNL;\n"
	"        else\n"
	"            light *= saturate(dot(N, normalize(D)));\n"
	"\n"
	"        lightingColour.xyz += light;\n"
	"    }\n"
	"\n"
	"    return lightingColour;\n"
	"}\n"
	"#endif//LIT\n"
	"\n"
	"void VSMain(const VSInput input, out PSInput output)\n"
	"{\n"
	"    output.pos = mul(input.pos, g_WVP);\n"
	"\n"
	"#ifdef LIT\n"
	"\n"
	"    float3 N = mul(input.normal, g_InvXposeW);\n"
	"    N = normalize(N);\n"
	"\n"
	"    float3 worldPos = mul(input.pos, g_W);\n"
	"\n"
	"    output.colour = GetLightingColour(worldPos, N) * g_constantColour * input.colour;\n"
	"\n"
	"#else//LIT\n"
	"\n"
	"    output.colour = g_constantColour * input.colour;\n"
	"\n"
	"#endif//LIT\n"
	"\n"
	"#ifdef TEXTURED\n"
	"\n"
	"    output.tex = input.tex;\n"
	"\n"
	"#endif//TEXTURED\n"
	"\n"
	"}\n"
	"\n"
	"void PSMain(const PSInput input, out PSOutput output)\n"
	"{\n"
	"    output.colour = input.colour;\n"
	"\n"
	"#ifdef TEXTURED\n"
	"\n"
	"    output.colour *= g_texture.Sample(g_sampler, input.tex);\n"
	"\n"
	"#endif//TEXTURED\n"
	"\n"
	"}\n";

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ub::Vertex_Pos3fColour4ub():
pos(0.f, 0.f, 0.f)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ub::Vertex_Pos3fColour4ub(const XMFLOAT3 &posArg, VertexColour colourArg) :
pos(posArg.x, posArg.y, posArg.z),
colour(colourArg)
{
}

Vertex_Pos3fColour4ub::Vertex_Pos3fColour4ub(const XMVECTOR &posArg, VertexColour colourArg)
{
	XMStoreFloat3((XMFLOAT3*)&pos, posArg);
	colour = colourArg;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ub[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ub, pos), D3D11_INPUT_PER_VERTEX_DATA, 0, },
	{"COLOUR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex_Pos3fColour4ub, colour), D3D11_INPUT_PER_VERTEX_DATA, 0,},
};

const UINT g_vertexDescSize_Pos3fColour4ub = sizeof g_aVertexDesc_Pos3fColour4ub / sizeof g_aVertexDesc_Pos3fColour4ub[0];

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubNormal3f::Vertex_Pos3fColour4ubNormal3f():
pos(0.f, 0.f, 0.f),
normal(0.f, 0.f, 0.f)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubNormal3f::Vertex_Pos3fColour4ubNormal3f(const XMFLOAT3 &posArg, VertexColour colourArg, const XMFLOAT3 &normalArg):
pos(posArg.x, posArg.y, posArg.z ),
colour(colourArg),
normal(normalArg.x, normalArg.y, normalArg.z)
{
}

Vertex_Pos3fColour4ubNormal3f::Vertex_Pos3fColour4ubNormal3f(const XMVECTOR &posArg, VertexColour colourArg, const XMVECTOR &normalArg) 
{
	XMStoreFloat3((XMFLOAT3*)&pos, posArg);
	colour = colourArg;
	XMStoreFloat3((XMFLOAT3*)&normal, normalArg);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

extern const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubNormal3f[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubNormal3f, pos), D3D11_INPUT_PER_VERTEX_DATA, 0, },
	{"COLOUR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex_Pos3fColour4ubNormal3f, colour), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubNormal3f, normal), D3D11_INPUT_PER_VERTEX_DATA, 0,},
};

extern const unsigned g_vertexDescSize_Pos3fColour4ubNormal3f = sizeof g_aVertexDesc_Pos3fColour4ubNormal3f / sizeof g_aVertexDesc_Pos3fColour4ubNormal3f[0];

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubTex2f::Vertex_Pos3fColour4ubTex2f():
pos(0.f, 0.f, 0.f),
tex(0.f, 0.f)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubTex2f::Vertex_Pos3fColour4ubTex2f(const XMFLOAT3 &posArg, VertexColour colourArg, const XMFLOAT2 &texArg) :
pos(posArg.x, posArg.y, posArg.z),
colour(colourArg),
tex(texArg.x, texArg.y)
{
}

Vertex_Pos3fColour4ubTex2f::Vertex_Pos3fColour4ubTex2f(const XMVECTOR &posArg, VertexColour colourArg, const XMVECTOR &texArg)
{
	XMStoreFloat3((XMFLOAT3*)&pos, posArg);
	colour = colourArg;
	XMStoreFloat2((XMFLOAT2*)&tex, texArg);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubTex2f[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubTex2f, pos), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"COLOUR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex_Pos3fColour4ubTex2f, colour), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubTex2f, tex), D3D11_INPUT_PER_VERTEX_DATA, 0,},
};

const UINT g_vertexDescSize_Pos3fColour4ubTex2f = sizeof g_aVertexDesc_Pos3fColour4ubTex2f / sizeof g_aVertexDesc_Pos3fColour4ubTex2f[0];

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubNormal3fTex2f::Vertex_Pos3fColour4ubNormal3fTex2f():
pos(0.f, 0.f, 0.f),
normal(0.f, 0.f, 0.f),
tex(0.f, 0.f)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Vertex_Pos3fColour4ubNormal3fTex2f::Vertex_Pos3fColour4ubNormal3fTex2f(const XMFLOAT3 &posArg, VertexColour colourArg, const XMFLOAT3 &normalArg, const XMFLOAT2 &texArg) :
pos(posArg.x, posArg.y, posArg.z),
colour(colourArg),
tex(texArg.x, texArg.y),
normal(normalArg.x, normalArg.y, normalArg.z)
{
}

Vertex_Pos3fColour4ubNormal3fTex2f::Vertex_Pos3fColour4ubNormal3fTex2f(const XMVECTOR &posArg, VertexColour colourArg, const XMVECTOR &normalArg, const XMFLOAT2 &texArg) :
colour(colourArg),
tex(texArg.x, texArg.y)
{
	XMStoreFloat3((XMFLOAT3*)&pos, posArg);
	XMStoreFloat3((XMFLOAT3*)&normal, normalArg);
}

Vertex_Pos3fColour4ubNormal3fTex2f::Vertex_Pos3fColour4ubNormal3fTex2f(const XMVECTOR &posArg, VertexColour colourArg, const XMVECTOR &normalArg, const XMVECTOR &texArg)
{
	XMStoreFloat3((XMFLOAT3*)&pos, posArg);
	colour = colourArg;
	XMStoreFloat2((XMFLOAT2*)&tex, texArg);
	XMStoreFloat3((XMFLOAT3*)&normal, normalArg);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const D3D11_INPUT_ELEMENT_DESC g_aVertexDesc_Pos3fColour4ubNormal3fTex2f[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubNormal3fTex2f, pos), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"COLOUR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex_Pos3fColour4ubNormal3fTex2f, colour), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubNormal3fTex2f, normal), D3D11_INPUT_PER_VERTEX_DATA, 0,},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex_Pos3fColour4ubNormal3fTex2f, tex), D3D11_INPUT_PER_VERTEX_DATA, 0,},
};

const UINT g_vertexDescSize_Pos3fColour4ubNormal3fTex2f = sizeof g_aVertexDesc_Pos3fColour4ubNormal3fTex2f / sizeof g_aVertexDesc_Pos3fColour4ubNormal3fTex2f[0];

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CommonApp::CommonApp()
{
	for (int i = 0; i < NUM_BLEND_STATES; ++i)
		m_apBlendStates[i] = NULL;

	for (int i = 0; i < NUM_DEPTH_STENCIL_STATES; ++i)
		m_apDepthStencilStates[i] = NULL;

	for (int i = 0; i < NUM_RASTERIZER_STATES; ++i)
		m_apRasterizerStates[i] = NULL;

	for (int i = 0; i < NUM_SAMPLER_STATES; ++i)
		m_apSamplerStates[i] = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CommonApp::~CommonApp()
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool CommonApp::HandleStart()
{
	char maxNumLightsValue[100];
	_snprintf_s(maxNumLightsValue, sizeof maxNumLightsValue, _TRUNCATE, "%d", MAX_NUM_LIGHTS);

	// Tex NO, Lit NO
	{
		const D3D_SHADER_MACRO aMacros[] = {
			{NULL},
		};

		if (!this->CompileShaderFromString(&m_shaderUntextured, g_aShader, aMacros, g_aVertexDesc_Pos3fColour4ub, g_vertexDescSize_Pos3fColour4ub))
			return false;
	}

	// Tex NO, Lit YES
	{
		const D3D_SHADER_MACRO aMacros[] = {
			{"MAX_NUM_LIGHTS", maxNumLightsValue},
			{"LIT",NULL},
			{NULL},
		};

		if (!this->CompileShaderFromString(&m_shaderUntexturedLit, g_aShader, aMacros, g_aVertexDesc_Pos3fColour4ubNormal3f, g_vertexDescSize_Pos3fColour4ubNormal3f))
			return false;
	}

	// Tex YES, Lit NO
	{
		const D3D_SHADER_MACRO aMacros[] = {
			{"TEXTURED",NULL},
			{NULL},
		};

		if (!this->CompileShaderFromString(&m_shaderTextured, g_aShader, aMacros, g_aVertexDesc_Pos3fColour4ubTex2f, g_vertexDescSize_Pos3fColour4ubTex2f))
			return false;
	}

	// Tex YES, Lit YES
	{
		const D3D_SHADER_MACRO aMacros[] = {
			{"MAX_NUM_LIGHTS", maxNumLightsValue},
			{"LIT",NULL},
			{"TEXTURED",NULL},
			{NULL},
		};

		if (!this->CompileShaderFromString(&m_shaderTexturedLit, g_aShader, aMacros, g_aVertexDesc_Pos3fColour4ubNormal3fTex2f, g_vertexDescSize_Pos3fColour4ubNormal3fTex2f))
			return false;
	}

	// Blend state
	for (int i = 0; i < NUM_BLEND_STATES; ++i)
	{
		D3D11_BLEND_DESC bd;

		bd.AlphaToCoverageEnable = FALSE;
		bd.IndependentBlendEnable = FALSE;

		bd.RenderTarget[0].BlendEnable = i & BLEND_STATE_BLEND_ENABLE ? TRUE : FALSE;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(m_pD3DDevice->CreateBlendState(&bd, &m_apBlendStates[i])))
			return false;
	}

	// Depth/Stencil state

	for (int i = 0; i < NUM_DEPTH_STENCIL_STATES; ++i)
	{
		D3D11_DEPTH_STENCIL_DESC dsd;

		dsd.DepthEnable = i & DEPTH_STENCIL_STATE_DEPTH_ENABLE ? TRUE : FALSE;
		dsd.DepthWriteMask = i & DEPTH_STENCIL_STATE_DEPTH_WRITE_ENABLE ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		dsd.DepthFunc = D3D11_COMPARISON_LESS;
		dsd.StencilEnable = FALSE;
		dsd.StencilReadMask = 0xFF;
		dsd.StencilWriteMask = 0xFF;

		dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

		dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

		if (FAILED(m_pD3DDevice->CreateDepthStencilState(&dsd, &m_apDepthStencilStates[i])))
			return false;
	}

	// Rasterizer state
	for (int i = 0; i < NUM_RASTERIZER_STATES; ++i)
	{
		D3D11_RASTERIZER_DESC rd;

		rd.FillMode = i & RASTERIZER_STATE_WIREFRAME ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
		rd.CullMode = i & RASTERIZER_STATE_BACK_FACE_CULL ? D3D11_CULL_BACK : D3D11_CULL_NONE;
		rd.FrontCounterClockwise = FALSE;
		rd.DepthBias = 0;
		rd.SlopeScaledDepthBias = 0.f;
		rd.DepthBiasClamp = 0.f;
		rd.DepthClipEnable = TRUE;
		rd.ScissorEnable = FALSE;
		rd.MultisampleEnable = FALSE;
		rd.AntialiasedLineEnable = FALSE;

		if (FAILED(m_pD3DDevice->CreateRasterizerState(&rd, &m_apRasterizerStates[i])))
			return false;
	}

	// Sampler states
	for (int i = 0; i < NUM_SAMPLER_STATES; ++i)
	{
		D3D11_SAMPLER_DESC sd;

		sd.Filter = i & SAMPLER_STATE_BILINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
		sd.AddressU = i & SAMPLER_STATE_WRAP ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressV = i & SAMPLER_STATE_WRAP ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressW = i & SAMPLER_STATE_WRAP ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.MipLODBias = 0.f;
		sd.MaxAnisotropy = 1;
		sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		memset(sd.BorderColor, 0, sizeof sd.BorderColor);
		sd.MinLOD = 0.f;
		sd.MaxLOD = i & SAMPLER_STATE_MIPMAP ? D3D11_FLOAT32_MAX : 0.f;

		if (FAILED(m_pD3DDevice->CreateSamplerState(&sd, &m_apSamplerStates[i])))
			return false;
	}

	// Set defaults
	this->SetBlendState(DEFAULT_BLEND_ENABLE);
	this->SetDepthStencilState(DEFAULT_DEPTH_TEST, DEFAULT_DEPTH_WRITE);
	this->SetRasterizerState(DEFAULT_BACK_FACE_CULL, DEFAULT_WIREFRAME);

	D3DXMatrixIdentity(&m_projectionMtx);
	D3DXMatrixIdentity(&m_viewMtx);
	D3DXMatrixIdentity(&m_worldMtx);

	m_constantColour=D3DXVECTOR4(1.f, 1.f, 1.f, 1.f);

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::HandleStop()
{
	for (int i = 0; i < NUM_BLEND_STATES; ++i)
		Release(m_apBlendStates[i]);

	for (int i = 0; i < NUM_DEPTH_STENCIL_STATES; ++i)
		Release(m_apDepthStencilStates[i]);

	for (int i = 0; i < NUM_RASTERIZER_STATES; ++i)
		Release(m_apRasterizerStates[i]);

	for (int i = 0; i < NUM_SAMPLER_STATES; ++i)
		Release(m_apSamplerStates[i]);

	m_shaderUntextured.Reset();
	m_shaderUntexturedLit.Reset();
	m_shaderTextured.Reset();
	m_shaderTexturedLit.Reset();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11Device *CommonApp::GetDevice() const
{
	return m_pD3DDevice;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11DeviceContext *CommonApp::GetDeviceContext() const
{
	return m_pD3DDeviceContext;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

float CommonApp::GetWindowAspectRatio() const
{
	RECT client;
	GetClientRect(m_hWnd, &client);

	if (client.right == 0 || client.bottom == 0)
		return 0.f;
	else
		return client.right / float(client.bottom);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::GetWindowSize(float *pWidth, float *pHeight) const
{
	RECT client;
	GetClientRect(m_hWnd, &client);

	*pWidth = float(client.right);
	*pHeight = float(client.bottom);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetWorldMatrix(const D3DXMATRIX &worldMtx)
{
	m_worldMtx = worldMtx;
}

void CommonApp::SetWorldMatrix(const XMMATRIX &worldMtx)
{
	XMFLOAT4X4 mWorld;
	XMStoreFloat4x4(&mWorld, worldMtx);
	m_worldMtx = *((D3DXMATRIX*)&mWorld);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetViewMatrix(const XMMATRIX &viewMtx)
{
	XMFLOAT4X4 mView;
	XMStoreFloat4x4(&mView, viewMtx);
	m_viewMtx = *((D3DXMATRIX*)&mView);
}

void CommonApp::SetViewMatrix(const D3DXMATRIX &viewMtx)
{
	m_viewMtx = viewMtx;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetProjectionMatrix(const XMMATRIX &projectionMtx)
{
	XMFLOAT4X4 mProj;
	XMStoreFloat4x4(&mProj, projectionMtx);
	m_projectionMtx = *((D3DXMATRIX*)&mProj);
}


void CommonApp::SetProjectionMatrix(const D3DXMATRIX &projectionMtx)
{
	m_projectionMtx = projectionMtx;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetDefaultProjectionMatrix(float aspect)
{
	D3DXMATRIX projMtx;
	D3DXMatrixPerspectiveFovLH(&projMtx, float(D3DX_PI / 4.f), aspect, 1.f, 250.f);

	this->SetProjectionMatrix(projMtx);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetDefaultViewMatrix(const D3DXVECTOR3 &camera, const D3DXVECTOR3 &lookat, const D3DXVECTOR3 &up)
{
	D3DXMATRIX viewMtx;
	D3DXMatrixLookAtLH(&viewMtx, &camera, &lookat, &up);

	this->SetViewMatrix(viewMtx);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DrawUntextured(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems)
{
	this->DrawWithShader(topology, pVertexBuffer, sizeof(Vertex_Pos3fColour4ub), pIndexBuffer, 0, numItems, NULL, NULL, &m_shaderUntextured);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems)
{
	this->DrawWithShader(topology, pVertexBuffer, sizeof(Vertex_Pos3fColour4ubNormal3f), pIndexBuffer, 0, numItems, NULL, NULL, &m_shaderUntexturedLit);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DrawTextured(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler)
{
	this->DrawWithShader(topology, pVertexBuffer, sizeof(Vertex_Pos3fColour4ubTex2f), pIndexBuffer, 0, numItems, pTextureView, pTextureSampler, &m_shaderTextured);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DrawTexturedLit(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, ID3D11Buffer *pIndexBuffer, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler)
{
	this->DrawWithShader(topology, pVertexBuffer, sizeof(Vertex_Pos3fColour4ubNormal3fTex2f), pIndexBuffer, 0, numItems, pTextureView, pTextureSampler, &m_shaderTexturedLit);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY topology, ID3D11Buffer *pVertexBuffer, size_t vertexStride, ID3D11Buffer *pIndexBuffer, unsigned firstItem, unsigned numItems, ID3D11ShaderResourceView *pTextureView, ID3D11SamplerState *pTextureSampler, Shader *pShader)
{
	if (pShader->pVSCBuffer || pShader->pPSCBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE vsMap;
		if (!pShader->pVSCBuffer || FAILED(m_pD3DDeviceContext->Map(pShader->pVSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vsMap)))
			vsMap.pData = NULL;

		D3D11_MAPPED_SUBRESOURCE psMap;
		if (!pShader->pPSCBuffer || FAILED(m_pD3DDeviceContext->Map(pShader->pPSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &psMap)))
			psMap.pData = NULL;

		// This bit of code does all the work, every time, relying on
		// on SetCBufferXXX to check if the given input value is
		// actually required by the shader or not.
		//
		// If doing this properly, one would be more careful about
		// not doing any unnecessary work. For example, if the
		// shader doesn't have a invXposeW constant, no need to
		// calculate it. Or if the shader doesn't have a numLights
		// constant, skip all the lighting setup. And so on.

		D3DXMATRIX wvp;
		this->GetWVP(&wvp);

		D3DXMATRIX invXposeW;
		if (D3DXMatrixInverse(&invXposeW, NULL, &m_worldMtx))
			D3DXMatrixTranspose(&invXposeW, &invXposeW);
		else
		{
			// Inversion failed... erm...
			invXposeW = m_worldMtx;
		}

		SetCBufferFloat4x4(vsMap, pShader->vsGlobals.wvp, wvp);
		SetCBufferFloat4x4(psMap, pShader->psGlobals.wvp, wvp);

		SetCBufferFloat4x4(vsMap, pShader->vsGlobals.w, m_worldMtx);
		SetCBufferFloat4x4(psMap, pShader->psGlobals.w, m_worldMtx);

		SetCBufferFloat4x4(vsMap, pShader->vsGlobals.invXposeW, invXposeW);
		SetCBufferFloat4x4(psMap, pShader->psGlobals.invXposeW, invXposeW);

		SetCBufferFloat4(vsMap, pShader->vsGlobals.constantColour, m_constantColour);
		SetCBufferFloat4(psMap, pShader->psGlobals.constantColour, m_constantColour);

		int numLights = 0;

		for (int i = 0; i < MAX_NUM_LIGHTS; ++i)
		{
			const Light *pLight = &m_lights[i];
			D3DXVECTOR4 direction, position, attenuations, spots;
			bool set = false;

			switch (pLight->type)
			{
			case Light::Type_Directional:
				{
					set = true;

					direction = D3DXVECTOR4(pLight->direction, 1.f);
					position = D3DXVECTOR4(0.f, 0.f, 0.f, 0.f);
					attenuations = D3DXVECTOR4(pLight->a0, pLight->a1, pLight->a2, pLight->rangeSquared);
					spots = D3DXVECTOR4(0.f, -1.f, 0.f, 0.f);
				}
				break;

			case Light::Type_Point:
				{
					set = true;

					direction = D3DXVECTOR4(0.f, 0.f, 0.f, 0.f);
					position = D3DXVECTOR4(pLight->position, 1.f);
					attenuations = D3DXVECTOR4(pLight->a0, pLight->a1, pLight->a2, pLight->rangeSquared);
					spots = D3DXVECTOR4(0.f, -1.f, 0.f, 0.f);
				}
				break;

			case Light::Type_Spot:
				{
					set = true;

					direction = D3DXVECTOR4(pLight->direction, 1.f);
					position = D3DXVECTOR4(pLight->position, 1.f);
					attenuations = D3DXVECTOR4(pLight->a0, pLight->a1, pLight->a2, pLight->rangeSquared);

					spots = D3DXVECTOR4(pLight->cosHalfPhi, pLight->cosHalfTheta, 0.f, pLight->falloff);

					if (pLight->cosHalfPhi != pLight->cosHalfTheta)
						spots.z = 1.f / (pLight->cosHalfTheta - pLight->cosHalfPhi);
				}
				break;
			}

			if (set)
			{
				SetCBufferArrayFloat4(vsMap, pShader->vsGlobals.lightDirections, numLights, direction);
				SetCBufferArrayFloat4(psMap, pShader->psGlobals.lightDirections, numLights, direction);

				SetCBufferArrayFloat4(vsMap, pShader->vsGlobals.lightPositions, numLights, position);
				SetCBufferArrayFloat4(psMap, pShader->psGlobals.lightPositions, numLights, position);

				SetCBufferArrayFloat3(vsMap, pShader->vsGlobals.lightColours, numLights, pLight->diffuseColour);
				SetCBufferArrayFloat3(psMap, pShader->psGlobals.lightColours, numLights, pLight->diffuseColour);

				SetCBufferArrayFloat4(vsMap, pShader->vsGlobals.lightAttenuations, numLights, attenuations);
				SetCBufferArrayFloat4(psMap, pShader->psGlobals.lightAttenuations, numLights, attenuations);

				SetCBufferArrayFloat4(vsMap, pShader->vsGlobals.lightSpots, numLights, spots);
				SetCBufferArrayFloat4(psMap, pShader->psGlobals.lightSpots, numLights, spots);

				++numLights;
			}
		}

		SetCBufferInt(psMap, pShader->psGlobals.numLights, numLights);
		SetCBufferInt(vsMap, pShader->vsGlobals.numLights, numLights);

		if (vsMap.pData)
			m_pD3DDeviceContext->Unmap(pShader->pVSCBuffer, 0);

		if (psMap.pData)
			m_pD3DDeviceContext->Unmap(pShader->pPSCBuffer, 0);

		if (pShader->pVSCBuffer)
		{
			ID3D11Buffer *apConstantBuffers[1] = {
				pShader->pVSCBuffer,
			};

			m_pD3DDeviceContext->VSSetConstantBuffers(pShader->vsGlobals.cbuffer, 1, apConstantBuffers);
		}

		if (pShader->pPSCBuffer)
		{
			ID3D11Buffer *apConstantBuffers[1] = {
				pShader->pPSCBuffer,
			};

			m_pD3DDeviceContext->PSSetConstantBuffers(pShader->psGlobals.cbuffer, 1, apConstantBuffers);
		}
	}

	// Set up vertex shader
	m_pD3DDeviceContext->VSSetShader(pShader->pVS, NULL, 0);

	// Set up pixel shader
	m_pD3DDeviceContext->PSSetShader(pShader->pPS, NULL, 0);

	if (pShader->psTexture >= 0)
	{
		ID3D11ShaderResourceView *apTextureViews[1] = {
			pTextureView,
		};

		m_pD3DDeviceContext->PSSetShaderResources(pShader->psTexture, 1, apTextureViews);
	}

	if (pShader->psSampler >= 0)
	{
		ID3D11SamplerState *apSamplerStates[1] = {
			pTextureSampler,
		};

		m_pD3DDeviceContext->PSSetSamplers(pShader->psSampler, 1, apSamplerStates);
	}

	// Draw
	m_pD3DDeviceContext->IASetPrimitiveTopology(topology);

	m_pD3DDeviceContext->IASetInputLayout(pShader->pIL);

	ID3D11Buffer *apVertexBuffers[1] = {
		pVertexBuffer,
	};
	UINT aStrides[1] = {
		vertexStride,
	};
	UINT aOffsets[1] = {
		0,
	};
	m_pD3DDeviceContext->IASetVertexBuffers(0, 1, apVertexBuffers, aStrides, aOffsets);

	if (pIndexBuffer)
	{
		m_pD3DDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		m_pD3DDeviceContext->DrawIndexed(numItems, firstItem, 0);
	}
	else
		m_pD3DDeviceContext->Draw(numItems, firstItem);

	if (pShader->psTexture >= 0)
	{
		// Strictly speaking, this isn't necessary. It makes use of render
		// targets a bit simpler though.

		ID3D11ShaderResourceView *apTextureViews[1] = {
			NULL,
		};

		m_pD3DDeviceContext->PSSetShaderResources(pShader->psTexture, 1, apTextureViews);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetConstantColour(const D3DXVECTOR4 &constantColour)
{
	m_constantColour = constantColour;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::DisableLight(int light)
{
	if (Light *pLight = this->GetLight(light))
		pLight->type = Light::Type_None;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::EnableDirectionalLight(int light, const XMFLOAT3 &worldDirection, const XMFLOAT3 &diffuseColour)
{
	if (Light *pLight = this->GetLight(light))
	{
		pLight->type = Light::Type_Directional;

		D3DXVec3Normalize(&pLight->direction, (D3DXVECTOR3*)&worldDirection);
		pLight->direction = -pLight->direction;

		pLight->a0 = 1.f;
		pLight->a1 = 0.f;
		pLight->a2 = 0.f;
		pLight->rangeSquared = FLT_MAX;

		pLight->diffuseColour = *((D3DXVECTOR3*)&diffuseColour);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::EnablePointLight(int light, const XMFLOAT3 &worldPosition, const XMFLOAT3 &diffuseColour)
{
	if (Light *pLight = this->GetLight(light))
	{
		pLight->type = Light::Type_Point;

		pLight->position = *((D3DXVECTOR3*)&worldPosition);

		pLight->a0 = 1.f;
		pLight->a1 = 0.f;
		pLight->a2 = 0.f;
		pLight->rangeSquared = FLT_MAX;

		pLight->diffuseColour = *((D3DXVECTOR3*)&diffuseColour);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::EnableSpotLight(int light, const XMFLOAT3 &worldPosition, const XMFLOAT3 &worldDirection, float theta, float phi, float falloff, const XMFLOAT3 &diffuseColour)
{
	if (Light *pLight = this->GetLight(light))
	{
		pLight->type = Light::Type_Spot;

		D3DXVec3Normalize(&pLight->direction, (D3DXVECTOR3*)&worldDirection);
		pLight->direction = -pLight->direction;

		pLight->position = *((D3DXVECTOR3*)&worldPosition);

		pLight->a0 = 1.f;
		pLight->a1 = 0.f;
		pLight->a2 = 0.f;
		pLight->rangeSquared = FLT_MAX;

		pLight->cosHalfTheta = cosf(theta * .5f);
		pLight->cosHalfPhi = cosf(phi * .5f);
		pLight->falloff = falloff;

		pLight->diffuseColour = *((D3DXVECTOR3*)&diffuseColour);
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetLightAttenuation(int light, float range, float a0, float a1, float a2)
{
	if (Light *pLight = this->GetLight(light))
	{
		pLight->a0 = a0;
		pLight->a1 = a1;
		pLight->a2 = a2;
		pLight->rangeSquared = range * range;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetBlendState(bool blendEnable)
{
	int i = 0;

	if (blendEnable)
		i |= BLEND_STATE_BLEND_ENABLE;

	m_pD3DDeviceContext->OMSetBlendState(m_apBlendStates[i], NULL, 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetDepthStencilState(bool depthTest, bool depthWrite)
{
	int i = 0;

	if (depthTest)
		i |= DEPTH_STENCIL_STATE_DEPTH_ENABLE;

	if (depthWrite)
		i |= DEPTH_STENCIL_STATE_DEPTH_WRITE_ENABLE;

	m_pD3DDeviceContext->OMSetDepthStencilState(m_apDepthStencilStates[i], 0);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::SetRasterizerState(bool backFaceCull, bool wireframe)
{
	int i = 0;

	if (backFaceCull)
		i |= RASTERIZER_STATE_BACK_FACE_CULL;

	if (wireframe)
		i |= RASTERIZER_STATE_WIREFRAME;

	m_pD3DDeviceContext->RSSetState(m_apRasterizerStates[i]);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

ID3D11SamplerState *CommonApp::GetSamplerState(bool bilinear, bool mipmap, bool wrap)
{
	int i = 0;

	if (bilinear)
		i |= SAMPLER_STATE_BILINEAR;

	if (mipmap)
		i |= SAMPLER_STATE_MIPMAP;

	if (wrap)
		i |= SAMPLER_STATE_WRAP;

	return m_apSamplerStates[i];
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::Clear(const XMFLOAT4 &clearColour)
{
	m_pD3DDeviceContext->ClearRenderTargetView(m_pD3DRenderTargetView, (float*)&clearColour);

	m_pD3DDeviceContext->ClearDepthStencilView(m_pD3DDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CommonApp::IsKeyPressed(int vkey) const
{
	if (this->IsInFocus())
	{
		SHORT val = GetAsyncKeyState(vkey);

		if (val & 0x8000)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::ShaderVars::ShaderVars():
cbuffer(-1),
wvp(-1),
invXposeW(-1),
w(-1),
constantColour(-1),
lightDirections(-1),
lightPositions(-1),
lightColours(-1),
lightAttenuations(-1),
lightSpots(-1),
numLights(-1)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader::Shader():
pVS(NULL),
pPS(NULL),
pIL(NULL),
pVSCBuffer(NULL),
pPSCBuffer(NULL)
{
	this->Reset();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader::~Shader()
{
	this->Reset();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::Shader::Reset()
{
	this->vsGlobals = ShaderVars();
	this->psGlobals = ShaderVars();

	this->psTexture = -1;
	this->psSampler = -1;

	Release(this->pPSCBuffer);
	Release(this->pVSCBuffer);
	Release(this->pIL);
	Release(this->pPS);
	Release(this->pVS);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool CommonApp::CompileShaderFromString(Shader *pShader, const char *pShaderCode, const D3D_SHADER_MACRO *pMacros, const D3D11_INPUT_ELEMENT_DESC *pInputElementsDescs, unsigned numInputElementsDescs)
{
	ID3D11VertexShader *pVS = NULL;
	ID3D11PixelShader *pPS = NULL;
	ID3D11InputLayout *pIL = NULL;
	ShaderDescription vs, ps;

	if (!CompileShadersFromString(m_pD3DDevice, pShaderCode, "VSMain", &pVS, &vs, pInputElementsDescs, numInputElementsDescs, &pIL, "PSMain", &pPS, &ps, pMacros))
		return false;

	this->CreateShaderFromCompiledShader(pShader, pVS, &vs, pIL, pPS, &ps);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool CommonApp::CompileShaderFromFile(Shader *pShader, const char *pShaderFileName, const D3D_SHADER_MACRO *pMacros, const D3D11_INPUT_ELEMENT_DESC *pInputElementsDescs, unsigned numInputElementsDescs)
{
	ID3D11VertexShader *pVS = NULL;
	ID3D11PixelShader *pPS = NULL;
	ID3D11InputLayout *pIL = NULL;
	ShaderDescription vs, ps;

	if (!CompileShadersFromFile(m_pD3DDevice, pShaderFileName, "VSMain", &pVS, &vs, pInputElementsDescs, numInputElementsDescs, &pIL, "PSMain", &pPS, &ps, pMacros))
		return false;

	this->CreateShaderFromCompiledShader(pShader, pVS, &vs, pIL, pPS, &ps);

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static void FindShaderVars(CommonApp::ShaderVars *pShaderVars, const ShaderDescription *pDescription)
{
	pDescription->FindCBuffer("CommonApp", &pShaderVars->cbuffer);

	pDescription->FindFloat4x4(pShaderVars->cbuffer, "g_WVP", &pShaderVars->wvp);
	pDescription->FindFloat4x4(pShaderVars->cbuffer, "g_InvXposeW", &pShaderVars->invXposeW);
	pDescription->FindFloat4x4(pShaderVars->cbuffer, "g_W", &pShaderVars->w);

	pDescription->FindFloat4(pShaderVars->cbuffer, "g_constantColour", &pShaderVars->constantColour);
	pDescription->FindFloat4(pShaderVars->cbuffer, "g_lightDirections", &pShaderVars->lightDirections);
	pDescription->FindFloat4(pShaderVars->cbuffer, "g_lightPositions", &pShaderVars->lightPositions);
	pDescription->FindFloat3(pShaderVars->cbuffer, "g_lightColours", &pShaderVars->lightColours);
	pDescription->FindFloat4(pShaderVars->cbuffer, "g_lightAttenuations", &pShaderVars->lightAttenuations);
	pDescription->FindFloat4(pShaderVars->cbuffer, "g_lightSpots", &pShaderVars->lightSpots);
	pDescription->FindInt(pShaderVars->cbuffer, "g_numLights", &pShaderVars->numLights);
}

void CommonApp::CreateShaderFromCompiledShader(Shader *pShader, ID3D11VertexShader *pVS, const ShaderDescription *pVSDescription, ID3D11InputLayout *pIL, ID3D11PixelShader *pPS, const ShaderDescription *pPSDescription)
{
	pShader->Reset();

	pShader->pVS = pVS;
	pShader->pIL = pIL;
	pShader->pPS = pPS;

	FindShaderVars(&pShader->vsGlobals, pVSDescription);
	FindShaderVars(&pShader->psGlobals, pPSDescription);

	pPSDescription->FindTexture("g_texture", &pShader->psTexture);
	pPSDescription->FindSamplerState("g_sampler", &pShader->psSampler);

	pShader->pVSCBuffer = CreateBuffer(m_pD3DDevice, pVSDescription->GetCBufferSizeBytes(pShader->vsGlobals.cbuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);
	pShader->pPSCBuffer = CreateBuffer(m_pD3DDevice, pPSDescription->GetCBufferSizeBytes(pShader->psGlobals.cbuffer), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

	// Should perhaps handle the error case, but it makes things easier
	// not to. The worst that will happen is that something won't get
	// drawn.
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader *CommonApp::GetUntexturedShader()
{
	return &m_shaderUntextured;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader *CommonApp::GetUntexturedLitShader()
{
	return &m_shaderUntexturedLit;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader *CommonApp::GetTexturedShader()
{
	return &m_shaderTextured;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Shader *CommonApp::GetTexturedLitShader()
{
	return &m_shaderTexturedLit;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Light::Light():
type(Type_None)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CommonApp::GetWVP(D3DXMATRIX *pWVP) const
{
	D3DXMATRIX vp;
	D3DXMatrixMultiply(&vp,&m_viewMtx,&m_projectionMtx);

	D3DXMatrixMultiply(pWVP,&m_worldMtx,&vp);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CommonApp::Light *CommonApp::GetLight(int light)
{
	if (light < 0 || light >= MAX_NUM_LIGHTS)
		return NULL;

	return &m_lights[light];
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
