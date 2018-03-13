#ifndef SPHERE_H
#define SPHERE_H

#include <DirectXMath.h>
#include <malloc.h>

class HeightMap;
class CommonMesh;


using namespace DirectX;



__declspec(align(16)) class Sphere
{
public:
	Sphere(XMVECTOR iSpherePos, 
		CommonMesh * i_pSphereMesh, HeightMap* i_pHeightMap) : mSpherePos(iSpherePos),
		m_pSphereMesh(i_pSphereMesh), m_pHeightMap(i_pHeightMap) 
	{
		XMFLOAT3 gravity = XMFLOAT3(0.0f, -45.0f, 0.0f);
		mGravityAcc = XMLoadFloat3(&gravity);

		mSphereVel = XMVectorZero();
	
	
	}
	~Sphere() {}

	void Update(float dT);
	void Draw();

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	XMVECTOR mSpherePos;
	XMVECTOR mSphereVel;

	float mMass = 1;
	float mInvMass = 1/mMass;
	float mRadius = 1;
	float mSphereSpeed;


	bool SphereTriangleIntersection(int nFaceIndex, XMVECTOR& colNormN, float& penetration);
	void TrianglePositionalCorrection(float penetration, XMVECTOR& colNormN);
	XMVECTOR ClosestPointToTriangle(XMVECTOR& a, XMVECTOR& b, XMVECTOR& c);
	bool CheckSphereTriCollisions(XMVECTOR& colPos, XMVECTOR& colNormN, float& penetration);

private:
	CommonMesh * m_pSphereMesh;
	XMMATRIX worldMtx;

	XMVECTOR mGravityAcc;
	HeightMap* m_pHeightMap;

	bool mSphereCollided = false;

	void BounceSphereTri(XMVECTOR colNormN);

};

#endif