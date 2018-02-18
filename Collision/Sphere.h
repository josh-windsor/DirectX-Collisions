#ifndef SPHERE_H
#define SPHERE_H

#include <DirectXMath.h>
#include <malloc.h>

class HeightMap;
class CommonMesh;


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

private:
	CommonMesh * m_pSphereMesh;
	DirectX::XMMATRIX worldMtx;

	DirectX::XMFLOAT3 mSphereVel;
	const DirectX::XMFLOAT3 mGravityAcc = DirectX::XMFLOAT3(0.0f, -0.05f, 0.0f);
	HeightMap* m_pHeightMap;

	float mSphereSpeed;
	bool mSphereCollided = false;
};

#endif