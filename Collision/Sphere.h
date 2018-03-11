#ifndef SPHERE_H
#define SPHERE_H

#include <DirectXMath.h>
#include <malloc.h>

class HeightMap;
class CommonMesh;

#define RESTITUTION 0.6
#define FRICTION 0.9

__declspec(align(16)) class Sphere
{
public:
	Sphere(DirectX::XMFLOAT3 iSpherePos, DirectX::XMFLOAT3 iSphereVel,
		CommonMesh * i_pSphereMesh, HeightMap* i_pHeightMap) : mSpherePos(iSpherePos),
		mSphereVel(iSphereVel), m_pSphereMesh(i_pSphereMesh), m_pHeightMap(i_pHeightMap) {}
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
	DirectX::XMFLOAT3 mSpherePos;
	DirectX::XMFLOAT3 mSphereVel;

	float mMass = 1;
	float mRadius = 1;
	float mSphereSpeed;
	void Bounce(DirectX::XMVECTOR surfaceNormal);
	void SpherePositionalCorrection(Sphere* sphere2, float penetration, DirectX::XMVECTOR& colNormN);
	bool SphereSphereIntersection(Sphere* sphere2, DirectX::XMVECTOR& colN, float& distance);
	bool SphereTriangleIntersection(int nFaceIndex, DirectX::XMVECTOR& colPos, DirectX::XMVECTOR& colNormN);
	void TrianglePositionalCorrection(float penetration, DirectX::XMVECTOR& colNormN);
	DirectX::XMVECTOR ClosestPointToTriangle(DirectX::XMVECTOR& a, DirectX::XMVECTOR& b, DirectX::XMVECTOR& c);
	bool CheckSphereTriCollisions(DirectX::XMVECTOR& colPos, DirectX::XMVECTOR& colNormN);

private:
	CommonMesh * m_pSphereMesh;
	DirectX::XMMATRIX worldMtx;

	DirectX::XMFLOAT3 mSphereVel;
	const DirectX::XMFLOAT3 mGravityAcc = DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f);
	HeightMap* m_pHeightMap;

	bool mSphereCollided = false;

};

#endif