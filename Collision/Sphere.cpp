
#include "Sphere.h"
using namespace DirectX;


#include "HeightMap.h"

void Sphere::Update(float dT)
{
	// Update Sphere
	XMVECTOR vSColPos, vSColNorm;

	if (!mSphereCollided)
	{
		XMVECTOR vSPos = XMLoadFloat3(&mSpherePos);
		XMVECTOR vSVel = XMLoadFloat3(&mSphereVel);
		XMVECTOR vSAcc = XMLoadFloat3(&mGravityAcc);

		vSPos += vSVel; // Really important that we add LAST FRAME'S velocity as this was how fast the collision is expecting the ball to move
		vSVel += vSAcc; // The new velocity gets passed through to the collision so it can base its predictions on our speed NEXT FRAME


		XMStoreFloat3(&mSphereVel, vSVel);
		XMStoreFloat3(&mSpherePos, vSPos);

		mSphereSpeed = XMVectorGetX(XMVector3Length(vSVel));

		mSphereCollided = m_pHeightMap->RayCollision(vSPos, vSVel, mSphereSpeed, vSColPos, vSColNorm);

		if (mSphereCollided)
		{
			mSphereVel = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMStoreFloat3(&mSpherePos, vSColPos);
		}
	}
}
void Sphere::Draw()
{
	if (m_pSphereMesh)
	{
		m_pSphereMesh->Draw();
	}
}
