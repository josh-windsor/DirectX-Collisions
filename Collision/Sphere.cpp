
#include "Sphere.h"
using namespace DirectX;


#include "HeightMap.h"

void Sphere::Update(float dT)
{
	// Update Sphere

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


	}
}

void Sphere::CheckCollision() 
{
	if (!mSphereCollided)
	{
		XMVECTOR vSColPos, vSColNorm;
		mSphereCollided = m_pHeightMap->SphereTriCollision(this, vSColPos, vSColNorm);

		if (mSphereCollided)
		{
			Bounce(vSColNorm);
			mSphereCollided = false;

			/*if (mSphereSpeed > 0.2f)
			{
			mSphereCollided = false;

			}
			else
			{
			mSphereVel = XMFLOAT3(0,0,0);
			}*/
		}
	}

}

void Sphere::Bounce(XMVECTOR surfaceNormal)
{
	XMVECTOR sphereVel = XMLoadFloat3(&mSphereVel);
	XMVECTOR u = (XMVector4Dot(sphereVel, surfaceNormal)/ XMVector4Dot(surfaceNormal, surfaceNormal)) * surfaceNormal;
	XMVECTOR w = sphereVel - u;

	XMStoreFloat3(&mSphereVel, (FRICTION * w) - (RESTITUTION * u));
}

void Sphere::Draw()
{
	if (m_pSphereMesh)
	{
		m_pSphereMesh->Draw();
	}
}

