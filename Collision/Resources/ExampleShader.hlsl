//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// You can get syntax highlighting for HLSL files:
//
// 1. Visit Tools|Options, Text Editor\File Extension
//
// 2. Enter `hlsl' in the Extension field, and select `Microsoft Visual
//    C++' from the Editor dropdown. Click Add.
//
// 3. Close and re-open any hlsl files you had open already.
//
// As you might guess, this makes Visual Studio interpret HLSL as C++.
// So it isn't quite perfect, and you get red underlines everywhere.
// On the bright side, you at least get brace matching, and syntax
// highlighting for comments.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// See the CommonApp comments for the names of the globals it looks for
// and sets automatically.
cbuffer CommonApp
{
	float4x4 g_WVP;
	float4 g_lightDirections[MAX_NUM_LIGHTS];
	float3 g_lightColours[MAX_NUM_LIGHTS];
	int g_numLights;
	float4x4 g_InvXposeW;
	float4x4 g_W;
};

// Add your own globals into your own cbuffer, or just make them
// global. The globals that are outside any explicit cbuffer go
// into a special cbuffer called "$Globals".
cbuffer MyApp
{
	float	g_frameCount;
	float3	g_waveOrigin;
}

struct VSInput
{
	float4 pos:POSITION;
	float4 colour:COLOUR0;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
};

struct PSInput
{
	float4 pos:SV_Position;
	float4 colour:COLOUR0;
	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
	float4 mat:COLOUR1;
};

struct PSOutput
{
	float4 colour:SV_Target;
};

Texture2D g_materialMap;
Texture2D g_texture0;
Texture2D g_texture1;
Texture2D g_texture2;

SamplerState g_sampler;

// ***********************************************************************************************
// Note: I've kept the same structure (inputs/putputs) as the shader from the pixel shader 
// tutorial to make it easier to reintegrate the collision and shader tutorials. Nonetheless, 
// I've pretty much trashed the functionality to achieve the debug rendering effect
// ***********************************************************************************************

void VSMain(const VSInput input, out PSInput output)
{
	output.pos = mul(input.pos, g_WVP);
	
	output.colour = input.colour;
	output.normal = input.normal;

	output.tex = input.tex;

}

void PSMain(const PSInput input, out PSOutput output)
{
	float4 colour = input.colour;

	//Add a bit of a grid
	if( input.tex.x <= 0.01f || input.tex.y <= 0.01f || input.tex.x >= 0.99f || input.tex.y >= 0.99f  )
		colour = float4( 1.0f, 1.0f, 1.0f, 1.0f );
	else
		colour = float4( 0.6f, 0.6f, 0.6f, 1.0f );

	if( input.colour.y == 0.0f )
		colour = float4( 1.0f, 0.0f, 0.0f, 1.0f );

	if( input.normal.x < 0.25 )
		colour = colour*0.95;

	output.colour.xyz = (float3)colour;

	output.colour.w = input.colour.w;
	output.colour.w = 0.5;

}

