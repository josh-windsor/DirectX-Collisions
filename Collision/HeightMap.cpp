#include "HeightMap.h"
#include "StaticOctree.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(char* filename, float gridSize, float heightRange)
{
	LoadHeightMap(filename, gridSize, heightRange);

	m_pHeightMapBuffer = NULL;

	m_pPSCBuffer = NULL;
	m_pVSCBuffer = NULL;

	m_HeightMapFaceCount = (m_HeightMapLength - 1)*(m_HeightMapWidth - 1) * 2;

	m_pFaceData = new FaceCollisionData[m_HeightMapFaceCount];

	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		m_pFaceData[f].m_bDisabled = false;
		m_pFaceData[f].m_bCollided = false;
	}

	m_HeightMapVtxCount = m_HeightMapFaceCount * 3;

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		m_pTextures[i] = NULL;
		m_pTextureViews[i] = NULL;
	}

	m_pSamplerState = NULL;

	m_pHeightMapBuffer = CreateDynamicVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3fTex2f * m_HeightMapVtxCount, 0);

	BuildCollisionData();
	RebuildVertexData();

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		LoadTextureFromFile(Application::s_pApp->GetDevice(), g_aTextureFileNames[i], &m_pTextures[i], &m_pTextureViews[i], &m_pSamplerState);
	}


	ReloadShader(); // This compiles the shader


	//builds the static octree of the heightmap
	XMFLOAT3 boundingBottomLeftFront;
	XMFLOAT3 boundingTopRightBack;
	bool firstLoop = true;

	//determines the bounding box of the octree
	for(int i = 0; i < m_HeightMapFaceCount; ++i)
	{
		XMFLOAT3 currentVertPos = m_pFaceData[i].m_vCenter;
		if (firstLoop)
		{
			boundingBottomLeftFront = currentVertPos;
			firstLoop = false;
		}

		if (currentVertPos.x < boundingBottomLeftFront.x)
		{
			boundingBottomLeftFront.x = currentVertPos.x;
		}
		if (currentVertPos.y < boundingBottomLeftFront.y)
		{
			boundingBottomLeftFront.y = currentVertPos.y;
		}
		if (currentVertPos.z < boundingBottomLeftFront.z)
		{
			boundingBottomLeftFront.z = currentVertPos.z;
		}
		if (currentVertPos.x > boundingTopRightBack.x)
		{
			boundingTopRightBack.x = currentVertPos.x;
		}
		if (currentVertPos.y > boundingTopRightBack.y)
		{
			boundingTopRightBack.y = currentVertPos.y;
		}
		if (currentVertPos.z > boundingTopRightBack.z)
		{
			boundingTopRightBack.z = currentVertPos.z;
		}


	}
	XMFLOAT3 f_centralP = (boundingBottomLeftFront + boundingTopRightBack) / 2;
	//creates an octree using the geometry
	m_pStaticOct = BuildStaticSubNode(f_centralP, fabs(boundingBottomLeftFront.x - boundingTopRightBack.x), 2);
	//adds all the vertices to the octree
	for (int i = 0; i < m_HeightMapFaceCount; ++i)
	{
		InputVertex inputVert;
		inputVert.index = i;
		inputVert.centralPosition = XMLoadFloat3(&m_pFaceData[i].m_vCenter);
		
		m_pStaticOct->AddNode(m_pStaticOct, &inputVert);
	}

}


void HeightMap::BuildCollisionData(void)
{
	int mapIndex = 0;
	int faceIndex = 0;

	XMVECTOR v0, v1, v2, v3;
	int i0, i1, i2, i3;

	// This is the unstripped method, I wouldn't recommend changing this to the stripped method for the collision assignment
	for (int l = 0; l < m_HeightMapLength; ++l)
	{
		for (int w = 0; w < m_HeightMapWidth; ++w)
		{
			if (w < m_HeightMapWidth - 1 && l < m_HeightMapLength - 1)
			{
				i0 = mapIndex;
				i1 = mapIndex + m_HeightMapWidth;
				i2 = mapIndex + 1;
				i3 = mapIndex + m_HeightMapWidth + 1;

				v0 = XMLoadFloat4(&m_pHeightMap[i0]);
				v1 = XMLoadFloat4(&m_pHeightMap[i1]);
				v2 = XMLoadFloat4(&m_pHeightMap[i2]);
				v3 = XMLoadFloat4(&m_pHeightMap[i3]);

				XMVECTOR vA = v0 - v1;
				XMVECTOR vB = v1 - v2;
				XMVECTOR vC = v3 - v1;

				XMVECTOR vN1, vN2;
				vN1 = XMVector3Cross(vA, vB);
				vN1 = XMVector3Normalize(vN1);

				vN2 = XMVector3Cross(vB, vC);
				vN2 = XMVector3Normalize(vN2);

				XMStoreFloat3(&m_pFaceData[faceIndex + 0].m_v0, v0);
				XMStoreFloat3(&m_pFaceData[faceIndex + 0].m_v1, v1);
				XMStoreFloat3(&m_pFaceData[faceIndex + 0].m_v2, v2);
				XMStoreFloat3(&m_pFaceData[faceIndex + 0].m_vNormal, vN1);
				XMStoreFloat3(&m_pFaceData[faceIndex + 0].m_vCenter, (v0 + v1 + v2) / 3);

				XMStoreFloat3(&m_pFaceData[faceIndex + 1].m_v0, v2);
				XMStoreFloat3(&m_pFaceData[faceIndex + 1].m_v1, v1);
				XMStoreFloat3(&m_pFaceData[faceIndex + 1].m_v2, v3);
				XMStoreFloat3(&m_pFaceData[faceIndex + 1].m_vNormal, vN2);
				XMStoreFloat3(&m_pFaceData[faceIndex + 1].m_vCenter, (v1 + v2 + v3) / 3);



				faceIndex += 2;
			}

			mapIndex++;
		}
	}
}



void HeightMap::RebuildVertexData(void)
{
	D3D11_MAPPED_SUBRESOURCE map;

	if (SUCCEEDED(Application::s_pApp->GetDeviceContext()->Map(m_pHeightMapBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
	{
		Vertex_Pos3fColour4ubNormal3fTex2f* pMapVtxs = (Vertex_Pos3fColour4ubNormal3fTex2f*)map.pData;

		int vtxIndex = 0;
		int mapIndex = 0;

		XMVECTOR v0, v1, v2, v3, v4, v5;
		float tX0, tY0, tX1, tY1, tX2, tY2, tX3, tY3;

		VertexColour c0, c1, c2, c3;
		XMVECTOR vN1, vN2;

		static VertexColour STANDARD_COLOUR(255, 255, 255, 255);
		static VertexColour COLLISION_COLOUR(255, 0, 0, 255);

		VertexColour CUBE_COLOUR;

		// This is the unstripped method, I wouldn't recommend changing this to the stripped method for the collision assignment
		for (int f = 0; f < m_HeightMapFaceCount; f += 2)
		{
			v0 = XMLoadFloat3(&m_pFaceData[f + 0].m_v0);
			v1 = XMLoadFloat3(&m_pFaceData[f + 0].m_v1);
			v2 = XMLoadFloat3(&m_pFaceData[f + 0].m_v2);
			v3 = XMLoadFloat3(&m_pFaceData[f + 1].m_v0);
			v4 = XMLoadFloat3(&m_pFaceData[f + 1].m_v1);
			v5 = XMLoadFloat3(&m_pFaceData[f + 1].m_v2);

			if (m_pFaceData[f + 0].m_bDisabled)
				v0 = v1 = v2 = XMVectorZero();

			if (m_pFaceData[f + 1].m_bDisabled)
				v3 = v4 = v5 = XMVectorZero();

			vN1 = XMLoadFloat3(&m_pFaceData[f + 0].m_vNormal);
			vN2 = XMLoadFloat3(&m_pFaceData[f + 1].m_vNormal);

			tX0 = 0.0f;
			tY0 = 0.0f;
			tX1 = 0.0f;
			tY1 = 1.0f;
			tX2 = 1.0f;
			tY2 = 0.0f;
			tX3 = 1.0f;
			tY3 = 1.0f;

			c0 = m_pFaceData[f + 0].m_bCollided ? COLLISION_COLOUR : STANDARD_COLOUR;
			c1 = m_pFaceData[f + 1].m_bCollided ? COLLISION_COLOUR : STANDARD_COLOUR;

			pMapVtxs[vtxIndex + 0] = Vertex_Pos3fColour4ubNormal3fTex2f(v0, c0, vN1, XMFLOAT2(tX0, tY0));
			pMapVtxs[vtxIndex + 1] = Vertex_Pos3fColour4ubNormal3fTex2f(v1, c0, vN1, XMFLOAT2(tX1, tY1));
			pMapVtxs[vtxIndex + 2] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, c0, vN1, XMFLOAT2(tX2, tY2));
			pMapVtxs[vtxIndex + 3] = Vertex_Pos3fColour4ubNormal3fTex2f(v3, c1, vN2, XMFLOAT2(tX2, tY2));
			pMapVtxs[vtxIndex + 4] = Vertex_Pos3fColour4ubNormal3fTex2f(v4, c1, vN2, XMFLOAT2(tX1, tY1));
			pMapVtxs[vtxIndex + 5] = Vertex_Pos3fColour4ubNormal3fTex2f(v5, c1, vN2, XMFLOAT2(tX3, tY3));

			vtxIndex += 6;
		}
	}

	Application::s_pApp->GetDeviceContext()->Unmap(m_pHeightMapBuffer, 0);
}


int HeightMap::DisableBelowLevel(float fYLevel)
{
	int nHidden = 0;

	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		if (m_pFaceData[f].m_v0.y < fYLevel && m_pFaceData[f].m_v1.y < fYLevel && m_pFaceData[f].m_v2.y < fYLevel)
		{
			m_pFaceData[f].m_bDisabled = true;
			nHidden++;
		}
	}

	return nHidden;
}

int HeightMap::EnableAll(void)
{
	int nHidden = 0;

	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		if (m_pFaceData[f].m_bDisabled == true)
		{
			m_pFaceData[f].m_bDisabled = false;
			nHidden++;
		}
	}

	return nHidden;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap()
{
	if (m_pHeightMap)
		delete m_pHeightMap;

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		Release(m_pTextures[i]);
		Release(m_pTextureViews[i]);
	}

	Release(m_pHeightMapBuffer);
	m_pStaticOct->CleanTree(m_pStaticOct);
	DeleteShader();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::Draw(float frameCount)
{

	D3DXMATRIX worldMtx;
	D3DXMatrixIdentity(&worldMtx);

	ID3D11DeviceContext* pContext = Application::s_pApp->GetDeviceContext();

	Application::s_pApp->SetWorldMatrix(worldMtx);

	// Fill in the `myGlobals' cbuffer.
	//
	// The D3D11_MAP_WRITE_DISCARD flag is best for performance, but
	// leaves the buffer contents indeterminate. The entire buffer
	// needs to be set up for each draw.
	//
	// (This is the reason you need to "your" globals
	// into your own cbuffer - the cbuffer set up by CommonApp is
	// mapped using D3D11_MAP_WRITE_DISCARD too. If you set "your"
	// values correctly by hand, they will likely disappear when
	// the CommonApp maps the buffer to set its own variables.)
	if (m_pPSCBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		if (SUCCEEDED(pContext->Map(m_pPSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		{
			// Set the buffer contents. There is only one variable to set in this case.
			SetCBufferFloat(map, m_psFrameCount, frameCount);
			pContext->Unmap(m_pPSCBuffer, 0);
		}
	}

	if (m_pPSCBuffer)
	{
		ID3D11Buffer *apConstantBuffers[] = {
			m_pPSCBuffer,
		};

		pContext->PSSetConstantBuffers(m_psCBufferSlot, 1, apConstantBuffers);
	}

	if (m_pVSCBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		if (SUCCEEDED(pContext->Map(m_pVSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		{
			// Set the buffer contents. There is only one variable to set
			// in this case.
			SetCBufferFloat(map, m_vsFrameCount, frameCount);

			pContext->Unmap(m_pVSCBuffer, 0);
		}
	}

	if (m_pVSCBuffer)
	{
		ID3D11Buffer *apConstantBuffers[] = {
			m_pVSCBuffer,
		};

		pContext->VSSetConstantBuffers(m_vsCBufferSlot, 1, apConstantBuffers);
	}


	if (m_psTexture0 >= 0)
		pContext->PSSetShaderResources(m_psTexture0, 1, &m_pTextureViews[0]);

	if (m_psTexture1 >= 0)
		pContext->PSSetShaderResources(m_psTexture1, 1, &m_pTextureViews[1]);

	if (m_psTexture2 >= 0)
		pContext->PSSetShaderResources(m_psTexture2, 1, &m_pTextureViews[2]);

	if (m_psMaterialMap >= 0)
		pContext->PSSetShaderResources(m_psMaterialMap, 1, &m_pTextureViews[3]);

	if (m_vsMaterialMap >= 0)
		pContext->VSSetShaderResources(m_vsMaterialMap, 1, &m_pTextureViews[3]);


	m_pSamplerState = Application::s_pApp->GetSamplerState(true, true, true);

	Application::s_pApp->DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pHeightMapBuffer, sizeof(Vertex_Pos3fColour4ubNormal3fTex2f),
		NULL, 0, m_HeightMapVtxCount, NULL, m_pSamplerState, &m_shader);
}

bool HeightMap::ReloadShader(void)
{
	DeleteShader();

	ID3D11VertexShader *pVS = NULL;
	ID3D11PixelShader *pPS = NULL;
	ID3D11InputLayout *pIL = NULL;
	ShaderDescription vs, ps;

	ID3D11Device* pDevice = Application::s_pApp->GetDevice();

	// When the CommonApp draw functions set any of the light arrays,
	// they assume that the arrays are of CommonApp::MAX_NUM_LIGHTS
	// in size. Using a shader compiler #define is an easy way to
	// get this value to the shader.

	char maxNumLightsValue[100];
	_snprintf_s(maxNumLightsValue, sizeof maxNumLightsValue, _TRUNCATE, "%d", CommonApp::MAX_NUM_LIGHTS);

	D3D_SHADER_MACRO aMacros[] = {
		{ "MAX_NUM_LIGHTS", maxNumLightsValue, },
	{ NULL },
	};

	if (!CompileShadersFromFile(pDevice, "./Resources/ExampleShader.hlsl", "VSMain", &pVS, &vs, g_aVertexDesc_Pos3fColour4ubNormal3fTex2f,
		g_vertexDescSize_Pos3fColour4ubNormal3fTex2f, &pIL, "PSMain", &pPS, &ps, aMacros))
	{

		return false;// false;
	}

	Application::s_pApp->CreateShaderFromCompiledShader(&m_shader, pVS, &vs, pIL, pPS, &ps);

	// Find cbuffer(s) for the globals that won't get set by the CommonApp
	// code. These are shader-specific; you have to know what you are
	// looking for, if you're going to set it...
	//
	// There are separate sets of constants for vertex and pixel shaders,
	// which need setting up separately. See the CommonApp code for
	// an example of doing both. In this case, we know that the vertex
	// shader doesn't do anything interesting, so we only check the pixel
	// shader constants.

	ps.FindCBuffer("MyApp", &m_psCBufferSlot);
	ps.FindFloat(m_psCBufferSlot, "g_frameCount", &m_psFrameCount);

	vs.FindCBuffer("MyApp", &m_vsCBufferSlot);
	vs.FindFloat(m_vsCBufferSlot, "g_frameCount", &m_vsFrameCount);

	ps.FindTexture("g_texture0", &m_psTexture0);
	ps.FindTexture("g_texture1", &m_psTexture1);
	ps.FindTexture("g_texture2", &m_psTexture2);
	ps.FindTexture("g_materialMap", &m_psMaterialMap);

	vs.FindTexture("g_materialMap", &m_vsMaterialMap);

	// Create the cbuffer, using the shader description to find out how
	// large it needs to be.
	m_pPSCBuffer = CreateBuffer(pDevice, ps.GetCBufferSizeBytes(m_psCBufferSlot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);
	m_pVSCBuffer = CreateBuffer(pDevice, vs.GetCBufferSizeBytes(m_vsCBufferSlot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

	return true;

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::DeleteShader()
{
	Release(m_pPSCBuffer);
	Release(m_pVSCBuffer);

	m_shader.Reset();
}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMap::LoadHeightMap(char* filename, float gridSize, float heightRange)
{
	FILE* filePtr;
	int error;
	int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;


	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_pHeightMap = new XMFLOAT4[m_HeightMapWidth * m_HeightMapLength];

	if (!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;


	// Read the image data into the height map.
	for (j = 0; j<m_HeightMapLength; j++)
	{
		for (i = 0; i<m_HeightMapWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_HeightMapWidth * j) + i;

			m_pHeightMap[index].x = (i - (((float)m_HeightMapWidth - 1) / 2))*gridSize;
			m_pHeightMap[index].y = (float)height / 6 * heightRange;
			m_pHeightMap[index].z = (j - (((float)m_HeightMapLength - 1) / 2))*gridSize;
			m_pHeightMap[index].w = 0;

			k += 3;
		}
	}


	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


bool HeightMap::PointOverQuad(XMVECTOR& vPos, XMVECTOR& v0, XMVECTOR& v1, XMVECTOR& v2)
{
	if (XMVectorGetX(vPos) < max(XMVectorGetX(v1), XMVectorGetX(v2)) && XMVectorGetX(vPos) > min(XMVectorGetX(v1), XMVectorGetX(v2)))
		if (XMVectorGetZ(vPos) < max(XMVectorGetZ(v1), XMVectorGetZ(v2)) && XMVectorGetZ(vPos) > min(XMVectorGetZ(v1), XMVectorGetZ(v2)))
			return true;

	return false;
}

int g_badIndex = 0;

bool HeightMap::RayCollision(XMVECTOR& rayPos, XMVECTOR rayDir, float raySpeed, XMVECTOR& colPos, XMVECTOR& colNormN)
{

	XMVECTOR v0, v1, v2, v3;
	int i0, i1, i2, i3;
	float colDist = 0.0f;

	// This resets the collision colouring
	for (int f = 0; f < m_HeightMapFaceCount; ++f)
		m_pFaceData[f].m_bCollided = false;

#ifdef COLOURTEST
	// This is just a piece of test code for the map colouring
	static float frame = 0;

	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		if ((int)frame%m_HeightMapFaceCount == f)
			m_pFaceData[f].m_bCollided = true;
	}

	RebuildVertexData();

	frame += 0.1f;

	// end of test code
#endif


	// This is a brute force solution that checks against every triangle in the heightmap
	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		//012 213
		if (!m_pFaceData[f].m_bDisabled && RayTriangle(f, rayPos, rayDir, colPos, colNormN, colDist))
		{
			// Needs to be >=0 
			if (colDist <= raySpeed && colDist >= 0.0f)
			{
				m_pFaceData[f].m_bCollided = true;
				RebuildVertexData();
				return true;
			}
		}
	}

	return false;
}

//bool HeightMap::SphereSphereCollision() 
//{
//
//}


/*bool HeightMap::SphereTriCollision(Sphere* sphere,  XMVECTOR& colPos, XMVECTOR& colNormN)
{
	XMVECTOR v0, v1, v2, v3;
	int i0, i1, i2, i3;

	// This resets the collision colouring
	for (int f = 0; f < m_HeightMapFaceCount; ++f)
		m_pFaceData[f].m_bCollided = false;

#ifdef COLOURTEST
	// This is just a piece of test code for the map colouring
	static float frame = 0;

	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		if ((int)frame%m_HeightMapFaceCount == f)
			m_pFaceData[f].m_bCollided = true;
	}

	RebuildVertexData();

	frame += 0.1f;

	// end of test code
#endif


	// This is a brute force solution that checks against every triangle in the heightmap
	for (int f = 0; f < m_HeightMapFaceCount; ++f)
	{
		//012 213
		if (!m_pFaceData[f].m_bDisabled && SpherePlane(f, sphere, colPos, colNormN))
		{
				m_pFaceData[f].m_bCollided = true;
				RebuildVertexData();
				return true;
			
		}
	}

	return false;

}*/

/*bool HeightMap::SpherePlane(int nFaceIndex, Sphere* sphere, XMVECTOR& colPos, XMVECTOR& colNormN)
{
	XMVECTOR vert0 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v0);
	XMVECTOR vert1 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v1);
	XMVECTOR vert2 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v2);

	// Step 1: Calculate |COLNORM| 
	// Note that the variable colNormN is passed through by reference as part of the function parameters so you can calculate and return it
	colNormN = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_vNormal);

	XMVECTOR sphereCenter = XMLoadFloat3(&sphere->mSpherePos);
	XMVECTOR p = ClosestPointToTriangle(sphereCenter, vert0, vert1, vert2);
	XMVECTOR v = p - sphereCenter;

	float vv = XMVectorGetX(XMVector3Dot(v,v));

	if (vv <= sphere->mRadius)
	{
		PositionalCorrection(sphere, 1 - vv, colNormN);
		return true;
	}

	return false;
}*/

/*void HeightMap::PositionalCorrection(Sphere* sphere, float penetration, XMVECTOR& colNormN)
{ 
	const float percent = 0.8; // usually 20% to 80%
	const float slop = 0.05; // usually 0.01 to 0.1
	XMVECTOR vecCorrection = max(penetration - slop, 0.0f) / sphere->mMass * penetration * colNormN;
	XMVECTOR vecSpherePos = XMLoadFloat3(&sphere->mSpherePos);
	XMVECTOR vecCorrected = vecSpherePos + vecCorrection;
	XMFLOAT3 correctionF;
	XMStoreFloat3(&correctionF, vecCorrected);
	sphere->mSpherePos = correctionF;
}*/

//built from RTCD p141
// a b & c = vert0,1,2
/*XMVECTOR HeightMap::ClosestPointToTriangle(XMVECTOR& spherePos, XMVECTOR& a, XMVECTOR& b, XMVECTOR& c)
{
	// Check if P in vertex region outside A
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ap = spherePos - a;

	float d1 = XMVectorGetX(XMVector3Dot(ab, ap));
	float d2 = XMVectorGetX(XMVector3Dot(ac, ap));

	if (d1 <= 0.0f && d2 <= 0.0f) 
	{
		return a;// barycentric coordinates (1,0,0)
	}

	// Check if P in vertex region outside B
	XMVECTOR bp = spherePos - b;
	float d3 = XMVectorGetX(XMVector3Dot(ab, bp));
	float d4 = XMVectorGetX(XMVector3Dot(ac, bp));
	if (d3 >= 0.0f && d4 <= d3) 
	{
		return b; // barycentric coordinates (0,1,0)
	}

	// Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		return a + v * ab; // barycentric coordinates (1-v,v,0)
	}
	// Check if P in vertex region outside C
	XMVECTOR cp = spherePos - c;
	float d5 = XMVectorGetX(XMVector3Dot(ab, cp));
	float d6 = XMVectorGetX(XMVector3Dot(ac, cp));
	if (d6 >= 0.0f && d5 <= d6)
	{ 
		return c; // barycentric coordinates (0,0,1)
	}


	// Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) 
	{
		float w = d2 / (d2 - d6);
		return a + w * ac; // barycentric coordinates (1-w,0,w)
	}
	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) 
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + w * (c - b); // barycentric coordinates (0,1-w,w)
	}
	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f-v-w

}*/




// Function:	rayTriangle
// Description: Tests a ray for intersection with a triangle
// Parameters:
//				vert0		First vertex of triangle 
//				vert1		Second vertex of triangle
//				vert3		Third vertex of triangle
//				rayPos		Start position of ray
//				rayDir		Direction of ray
//				colPos		Position of collision (returned)
//				colNormN	The normalised Normal to triangle (returned)
//				colDist		Distance from rayPos to collision (returned)
// Returns: 	true if the intersection point lies within the bounds of the triangle.
// Notes: 		Not for the faint-hearted :)

bool HeightMap::RayTriangle(int nFaceIndex, const XMVECTOR& rayPos, const XMVECTOR& rayDir, XMVECTOR& colPos, XMVECTOR& colNormN, float& colDist)
{
	// Part 1: Calculate the collision point between the ray and the plane on which the triangle lies
	//
	// If RAYPOS is a point in space and RAYDIR is a vector extending from RAYPOS towards a plane
	// Then COLPOS with the plane will be RAYPOS + COLDIST*|RAYDIR|
	// So if we can calculate COLDIST then we can calculate COLPOS
	//
	// The equation for plane is Ax + By + Cz + D = 0
	// Which can also be written as [ A,B,C ] dot [ x,y,z ] = -D
	// Where [ A,B,C ] is |COLNORM| (the normalised normal to the plane) and [ x,y,z ] is any point on that plane 
	// Any point includes the collision point COLPOS which equals  RAYPOS + COLDIST*|RAYDIR|
	// So substitute [ x,y,z ] for RAYPOS + COLDIST*|RAYDIR| and rearrange to yield COLDIST
	// -> |COLNORM| dot (RAYPOS + COLDIST*|RAYDIR|) also equals -D
	// -> (|COLNORM| dot RAYPOS) + (|COLNORM| dot (COLDIST*|RAYDIR|)) = -D
	// -> |COLNORM| dot (COLDIST*|RAYDIR|)) = -D -(|COLNORM| dot RAYPOS)
	// -> COLDIST = -(D+(|COLNORM| dot RAYPOS)) /  (|COLNORM| dot |RAYDIR|)
	//
	// Now all we only need to calculate D in order to work out COLDIST
	// This can be done using |COLNORM| (which remember is also [ A,B,C ] ), the plane equation and any point on the plane
	// |COLNORM| dot |ANYVERT| = -D

	XMVECTOR vert0 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v0);
	XMVECTOR vert1 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v1);
	XMVECTOR vert2 = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_v2);

	// Step 1: Calculate |COLNORM| 
	// Note that the variable colNormN is passed through by reference as part of the function parameters so you can calculate and return it
	colNormN = XMLoadFloat3(&m_pFaceData[nFaceIndex].m_vNormal);

	// Next line is useful debug code to stop collision with the top of the inverted pyramid (which has a normal facing straight up). 
	//if( fabs(colNormN.m128_f32[1])>0.99f ) return false;
	// Remember to remove it once you have implemented part 2 below...

	// ...

	// Step 2: Use |COLNORM| and any vertex on the triangle to calculate D
	XMVECTOR vecD = -XMVector3Dot(colNormN, vert0);
	float D = 0;
	XMStoreFloat(&D, vecD);
	// ...
	XMVECTOR rayDirNormalized = XMVector3Normalize(rayDir);
	// Step 3: Calculate the demoninator of the COLDIST equation: (|COLNORM| dot |RAYDIR|) and "early out" (return false) if it is 0
	XMVECTOR vecDDenominator = -XMVector3Dot(colNormN, rayDirNormalized);
	float dDenominator = 0;
	XMStoreFloat(&dDenominator, vecDDenominator);
	if (dDenominator == 0) return false;

	// ...
	// Step 4: Calculate the numerator of the COLDIST equation: -(D+(|COLNORM| dot RAYPOS))
	XMVECTOR vecColrayDot = XMVector3Dot(colNormN, rayPos);
	float colrayDot = 0;
	XMStoreFloat(&colrayDot, vecColrayDot);
	float dNumerator = -(D + colrayDot);

	// ...


	// Step 5: Calculate COLDIST and "early out" again if COLDIST is behind RAYDIR
	colDist = dNumerator / dDenominator;
	// ...

	// Step 6: Use COLDIST to calculate COLPOS-- RAYPOS + COLDIST*|RAYDIR|
	colPos = rayPos + colDist * rayDirNormalized;
	// ...

	// Next two lines are useful debug code to stop collision with anywhere beneath the pyramid. 
	// if( min(vert0.y,vert1.y,vert2.y)>colPos.y ) return false;
	// Remember to remove it once you have implemented part 2 below...

	// Part 2: Work out if the intersection point falls within the triangle
	//
	// If the point is inside the triangle then it will be contained by the three new planes defined by:
	// 1) RAYPOS, VERT0, VERT1
	// 2) RAYPOS, VERT1, VERT2
	// 3) RAYPOS, VERT2, VERT0

	// Move the ray backwards by a tiny bit (one unit) in case the ray is already on the plane

	XMVECTOR colPosAdjusted = colPos - (2 * rayDirNormalized);
	// ...

	// Step 1: Test against plane 1 and return false if behind plane
	if (!PointPlane(rayPos, vert0, vert1, colPosAdjusted))
	{
		return false;
	}
	// ...

	// Step 2: Test against plane 2 and return false if behind plane
	if (!PointPlane(rayPos, vert1, vert2, colPosAdjusted))
	{
		return false;
	}

	// ...

	// Step 3: Test against plane 3 and return false if behind plane
	if (!PointPlane(rayPos, vert2, vert0, colPosAdjusted))
	{
		return false;
	}

	// ...

	// Step 4: Return true! (on triangle)
	return true;
}

// Function:	pointPlane
// Description: Tests a point to see if it is in front of a plane
// Parameters:
//				vert0		First point on plane 
//				vert1		Second point on plane 
//				vert3		Third point on plane 
//				pointPos	Point to test
// Returns: 	true if the point is in front of the plane

bool HeightMap::PointPlane(const XMVECTOR& vert0, const XMVECTOR& vert1, const XMVECTOR& vert2, const XMVECTOR& pointPos)
{
	// For any point on the plane [x,y,z] Ax + By + Cz + D = 0
	// So if Ax + By + Cz + D < 0 then the point is behind the plane
	// --> [ A,B,C ] dot [ x,y,z ] + D < 0
	// --> |PNORM| dot POINTPOS + D < 0
	// but D = -(|PNORM| dot VERT0 )
	// --> (|PNORM| dot POINTPOS) - (|PNORM| dot VERT0) < 0
	XMVECTOR sVec0, sVec1, sNormN;
	float sD, sNumer;

	// Step 1: Calculate PNORM
	sVec0 = vert1 - vert0;
	sVec1 = vert2 - vert0;
	sNormN = XMVector3Normalize(XMVector3Cross(sVec0, sVec1));
	// ...

	// Step 2: Calculate D
	XMVECTOR vD = -XMVector3Dot(sNormN, vert0);
	XMStoreFloat(&sD, vD);
	// ...

	// Step 3: Calculate full equation
	XMVECTOR vN = XMVector3Dot(sNormN, pointPos);
	XMStoreFloat(&sNumer, vN);
	// ...h

	// Step 4: Return false if < 0 (behind plane)
	if (sNumer + sD < 0)
	{
		return false;
	}
	// ...

	// Step 5: Return true! (in front of plane)
	return true;
}

StaticOctree* HeightMap::BuildStaticSubNode(XMFLOAT3 iCenterPoint, float iSideLength, int iMaxDepth)
{
	if (iMaxDepth >= 0)
	{
		StaticOctree* newNode = new StaticOctree(iCenterPoint, iSideLength);

		XMFLOAT3 offset;
		float step = iSideLength * 0.5f;

		for (int i = 0; i < 8; ++i)
		{
			offset.x = ((i & 1)) ? step : -step;
			offset.y = ((i & 2)) ? step : -step;
			offset.z = ((i & 4)) ? step : -step;

			newNode->children[i] = BuildStaticSubNode(iCenterPoint + offset, step, iMaxDepth - 1);
		}
		return newNode;
	}
	return nullptr;
}
