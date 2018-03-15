
#include "Sphere.h"
using namespace DirectX;


#include "HeightMap.h"

void Sphere::StartSphere(XMVECTOR iSpherePos) 
{
	mSphereAlive = true;
	mSpherePos = iSpherePos;
	mSphereVel = XMVectorZero();
}


void Sphere::Update(float dT)
{

	// Update Sphere
	if (mSphereAlive && !mSphereCollided)
	{

		XMVECTOR vSColPos, vSColNorm;

		mSphereVel += mGravityAcc * dT; // The new velocity gets passed through to the collision so it can base its predictions on our speed NEXT FRAME
		mSpherePos += mSphereVel * dT; // Really important that we add LAST FRAME'S velocity as this was how fast the collision is expecting the ball to move



		mSphereSpeed = XMVectorGetX(XMVector3Length(mSphereVel));
		float penetration;
		mSphereCollided = CheckSphereTriCollisions(vSColPos, vSColNorm, penetration);

		if (mSphereCollided)
		{

			//TrianglePositionalCorrection(penetration, vSColNorm);

			BounceSphereTri(vSColNorm);
			TrianglePositionalCorrection(penetration, vSColNorm);
			

			mSphereCollided = false;

		}
	}

}

bool Sphere::CheckSphereTriCollisions(XMVECTOR& colPos, XMVECTOR& colNormN, float& penetration)
{

	// This resets the collision coloring
	for (int f = 0; f < m_pHeightMap->m_HeightMapFaceCount; ++f)
		m_pHeightMap->m_pFaceData[f].m_bCollided = false;


	// This is a brute force solution that checks against every triangle in the heightmap
	for (int f = 0; f < m_pHeightMap->m_HeightMapFaceCount; ++f)
	{
		//012 213
		if (!m_pHeightMap->m_pFaceData[f].m_bDisabled && SphereTriangleIntersection(f, colNormN, penetration))
		{
			m_pHeightMap->m_pFaceData[f].m_bCollided = true;
			m_pHeightMap->RebuildVertexData();
			return true;

		}
	}

	return false;

}


bool Sphere::SphereTriangleIntersection(int nFaceIndex, XMVECTOR& collisionNormal, float& penetration)
{


	XMVECTOR vert0 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v0);
	XMVECTOR vert1 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v1);
	XMVECTOR vert2 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v2);

	collisionNormal = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_vNormal);

	XMVECTOR p = ClosestPointToTriangle(vert0, vert1, vert2);
	XMVECTOR v = p - mSpherePos;

	float distance = XMVectorGetX(XMVector3Dot(v, v));

	if (distance <= mRadius * mRadius)
	{
		penetration = mRadius - sqrt(distance);

		return true;
	}

	return false;
}

void Sphere::BounceSphereTri(XMVECTOR colNormN)
{
	float normalVelocityLength = XMVectorGetX(XMVector3Dot(-mSphereVel, colNormN));
	if (normalVelocityLength < 0.0f)
	{
		return;
	}
	float push = (-1.6f * normalVelocityLength);
	mSphereVel -= push * colNormN;
	
	XMVECTOR frictionT = -mSphereVel - (colNormN * XMVectorGetX(XMVector3Dot(-mSphereVel, colNormN)));

	float frictionTLength = XMVectorGetX(XMVector3LengthSq(frictionT));
	//the tangential speed is greater than 0 then we must apply friction impulse
	if (frictionTLength > 0.0000001f)
	{
		frictionTLength = sqrtf(frictionTLength);
		//friction normal to use in the impulse equation. Use the
		//minus sign so that it points in the opposite direction
		//of the tangential velocity vector
		XMVECTOR frictionNormal = -frictionT / frictionTLength;
		//the friction impulse. the tangential speed is the numerator
		float frictionImpulse = frictionTLength / mInvMass;

		//as when working with forces, if the friction impulse
		//is greater than the normal impulse times the static
		//friction coefficient, which means a slide, the
		//friction impulse is the normal impulse times the
		//dynamic friction coefficient else there's no change
		//to do there
		static float staticFriction = 0.3f;
		static float dynamicFriction = 0.5f;
		if (frictionImpulse > push * staticFriction)
		{
			frictionImpulse = push * dynamicFriction;
		}
		
		XMVECTOR frictionImpulseVector = frictionNormal * frictionImpulse;

		mSphereVel += frictionImpulseVector;
		
	}

}

void Sphere::TrianglePositionalCorrection(float penetration, XMVECTOR& colNormN)
{
	const float percent = 0.4f;
	const float slop = 0.001f;
	XMVECTOR vecCorrection = max(penetration - slop, 0.0f) / percent * colNormN;

	mSpherePos += vecCorrection;


}


XMVECTOR Sphere::ClosestPointToTriangle(XMVECTOR& a, XMVECTOR& b, XMVECTOR& c)
{

	// Check if P in vertex region outside A
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ap = mSpherePos - a;

	float d1 = XMVectorGetX(XMVector3Dot(ab, ap));
	float d2 = XMVectorGetX(XMVector3Dot(ac, ap));

	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		return a;// barycentric coordinates (1,0,0)
	}

	// Check if P in vertex region outside B
	XMVECTOR bp = mSpherePos - b;
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
	XMVECTOR cp = mSpherePos - c;
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

}




void Sphere::Draw()
{
	if (mSphereAlive && m_pSphereMesh)
	{
		m_pSphereMesh->Draw();
	}
}

