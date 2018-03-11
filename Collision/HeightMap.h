#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

//**********************************************************************
// File:			HeightMap.h
// Description:		A simple class to represent a heightmap
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2015
// Notes:			
//**********************************************************************

#include "Application.h"

static const char *const g_aTextureFileNames[] = {
	"Resources/Intersection.dds",
	"Resources/Intersection.dds",
	"Resources/Collision.dds",
	"Resources/MaterialMap.dds",
};

static const size_t NUM_TEXTURE_FILES = sizeof g_aTextureFileNames / sizeof g_aTextureFileNames[0];

class HeightMap
{
public:
	HeightMap(char* filename, float gridSize, float heightRange);
	~HeightMap();

	void Draw(float frameCount);
	bool ReloadShader();
	void DeleteShader();
	bool RayCollision(XMVECTOR& rayPos, XMVECTOR rayDir, float speed, XMVECTOR& colPos, XMVECTOR& colNormN);
	//bool SphereTriCollision(Sphere* sphere, XMVECTOR& colPos, XMVECTOR& colNormN);
	int DisableBelowLevel(float fY);
	int EnableAll(void);

	void GetTriangle(int index, XMFLOAT3* collisionData[5])
	{
		collisionData[0] = &m_pFaceData[index].m_v0;
		collisionData[1] = &m_pFaceData[index].m_v1;
		collisionData[2] = &m_pFaceData[index].m_v2;
		collisionData[3] = &m_pFaceData[index].m_vNormal;
		collisionData[4] = &m_pFaceData[index].m_vCenter;

	}

	struct FaceCollisionData
	{
		XMFLOAT3 m_v0;
		XMFLOAT3 m_v1;
		XMFLOAT3 m_v2;
		XMFLOAT3 m_vNormal;
		XMFLOAT3 m_vCenter;
		bool m_bCollided; // Debug colouring
		bool m_bDisabled;
	};
	FaceCollisionData* m_pFaceData;
	int m_HeightMapFaceCount;
	void RebuildVertexData(void);


private:



	bool LoadHeightMap(char* filename, float gridSize, float heightRange);
	bool RayTriangle(int nFaceIndex, const XMVECTOR& rayPos, const XMVECTOR& rayDir, XMVECTOR& colPos, XMVECTOR& colNormN, float& colDist);
	bool PointPlane(const XMVECTOR& vert0, const XMVECTOR& vert1, const XMVECTOR& vert2, const XMVECTOR& pointPos);
	//bool SpherePlane(int nFaceIndex, Sphere* sphere, XMVECTOR& colPos, XMVECTOR& colNormN);
	bool PointOverQuad(XMVECTOR& vPos, XMVECTOR& v0, XMVECTOR& v1, XMVECTOR& v2);
	void BuildCollisionData(void);

	//void PositionalCorrection(Sphere* sphere, float penetration, XMVECTOR& colNormN);

	//XMVECTOR ClosestPointToTriangle(XMVECTOR& spherePos, XMVECTOR& a, XMVECTOR& b, XMVECTOR& c);



	XMFLOAT3 GetFaceNormal(int faceIndex, int offset);
	XMFLOAT3 GetAveragedVertexNormal(int index, int row);

	ID3D11Buffer *m_pHeightMapBuffer;

	int m_HeightMapWidth;
	int m_HeightMapLength;
	int m_HeightMapVtxCount;
	XMFLOAT4* m_pHeightMap;
	Vertex_Pos3fColour4ubNormal3fTex2f* m_pMapVtxs;

	Application::Shader m_shader;

	ID3D11Buffer *m_pPSCBuffer;
	ID3D11Buffer *m_pVSCBuffer;

	int m_psCBufferSlot;
	int m_psFrameCount;

	int m_psTexture0;
	int m_psTexture1;
	int m_psTexture2;
	int m_psMaterialMap;
	int m_vsMaterialMap;

	int m_vsCBufferSlot;
	int m_vsFrameCount;

	ID3D11Texture2D *m_pTextures[NUM_TEXTURE_FILES];
	ID3D11ShaderResourceView *m_pTextureViews[NUM_TEXTURE_FILES];
	ID3D11SamplerState *m_pSamplerState;

};

#endif