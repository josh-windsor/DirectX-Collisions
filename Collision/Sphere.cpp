
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

		vSVel += vSAcc * dT; // The new velocity gets passed through to the collision so it can base its predictions on our speed NEXT FRAME
		vSPos += vSVel * dT; // Really important that we add LAST FRAME'S velocity as this was how fast the collision is expecting the ball to move


		XMStoreFloat3(&mSphereVel, vSVel);
		XMStoreFloat3(&mSpherePos, vSPos);

		mSphereSpeed = XMVectorGetX(XMVector3Length(vSVel));

		mSphereCollided = CheckSphereTriCollisions(vSColPos, vSColNorm);
//		mSphereCollided = CheckSphereSphereCollisions(vSColPos, vSColNorm);

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

bool Sphere::CheckSphereTriCollisions(XMVECTOR& colPos, XMVECTOR& colNormN)
{
	XMVECTOR v0, v1, v2, v3;
	int i0, i1, i2, i3;

	// This resets the collision coloring
	for (int f = 0; f < m_pHeightMap->m_HeightMapFaceCount; ++f)
		m_pHeightMap->m_pFaceData[f].m_bCollided = false;


	// This is a brute force solution that checks against every triangle in the heightmap
	for (int f = 0; f < m_pHeightMap->m_HeightMapFaceCount; ++f)
	{
		//012 213
		if (!m_pHeightMap->m_pFaceData[f].m_bDisabled && SphereTriangleIntersection(f, colPos, colNormN))
		{
			m_pHeightMap->m_pFaceData[f].m_bCollided = true;
			m_pHeightMap->RebuildVertexData();
			return true;

		}
	}

	return false;

}

bool Sphere::SphereSphereIntersection(Sphere* sphere2, XMVECTOR& colN, float& distance)
{
	XMVECTOR vecSphere1Pos = XMLoadFloat3(&mSpherePos);
	XMVECTOR vecSphere2Pos = XMLoadFloat3(&sphere2->mSpherePos);
	colN = vecSphere2Pos - vecSphere1Pos;

	distance = XMVectorGetX(XMVector3LengthSq(colN));

	float radius = mRadius + sphere2->mRadius;
	radius = radius * radius;

	if (distance < radius)
	{
		return true;
	}

	return false;

}

bool Sphere::SphereTriangleIntersection(int nFaceIndex, XMVECTOR& colPos, XMVECTOR& colNormN)
{
	XMVECTOR vert0 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v0);
	XMVECTOR vert1 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v1);
	XMVECTOR vert2 = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_v2);

	// Step 1: Calculate |COLNORM| 
	// Note that the variable colNormN is passed through by reference as part of the function parameters so you can calculate and return it
	colNormN = XMLoadFloat3(&m_pHeightMap->m_pFaceData[nFaceIndex].m_vNormal);

	XMVECTOR sphereCenter = XMLoadFloat3(&mSpherePos);
	XMVECTOR p = ClosestPointToTriangle(vert0, vert1, vert2);
	XMVECTOR v = p - sphereCenter;

	float vv = XMVectorGetX(XMVector3Dot(v, v));

	if (vv <= mRadius)
	{
		TrianglePositionalCorrection(1 - vv, colNormN);
		return true;
	}

	return false;
}

void Sphere::TrianglePositionalCorrection(float penetration, XMVECTOR& colNormN)
{
	const float percent = 0.99; // usually 20% to 80%
	const float slop = 0.001; // usually 0.01 to 0.1
	XMVECTOR vecCorrection = max(penetration - slop, 0.0f) / mMass * penetration * colNormN;
	XMVECTOR vecSpherePos = XMLoadFloat3(&mSpherePos);
	XMVECTOR vecCorrected = vecSpherePos + vecCorrection;
	XMFLOAT3 correctionF;
	XMStoreFloat3(&correctionF, vecCorrected);
	mSpherePos = correctionF;
}


void Sphere::SpherePositionalCorrection(Sphere* sphere2, float penetration, XMVECTOR& colNormN)
{
	const float percent = 0.6; // usually 20% to 80%
	const float slop = 0.01; // usually 0.01 to 0.1
	XMVECTOR vecCorrection = max(penetration - slop, 0.0f) / (mMass + sphere2->mMass) * penetration * colNormN;
	XMVECTOR vecSpherePos = XMLoadFloat3(&mSpherePos);
	XMVECTOR vecSpherePos2 = XMLoadFloat3(&sphere2->mSpherePos);

	XMVECTOR vecCorrected = vecSpherePos - vecCorrection;
	XMVECTOR vecCorrected2 = vecSpherePos2 + vecCorrection;

	XMFLOAT3 correctionF, correctionF2;
	XMStoreFloat3(&correctionF, vecCorrected);
	XMStoreFloat3(&correctionF2, vecCorrected2);

	mSpherePos = correctionF;
	sphere2->mSpherePos = correctionF2;

}

XMVECTOR Sphere::ClosestPointToTriangle(XMVECTOR& a, XMVECTOR& b, XMVECTOR& c)
{
	XMVECTOR vecSpherePos = XMLoadFloat3(&mSpherePos);

	// Check if P in vertex region outside A
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ap = vecSpherePos - a;

	float d1 = XMVectorGetX(XMVector3Dot(ab, ap));
	float d2 = XMVectorGetX(XMVector3Dot(ac, ap));

	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		return a;// barycentric coordinates (1,0,0)
	}

	// Check if P in vertex region outside B
	XMVECTOR bp = vecSpherePos - b;
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
	XMVECTOR cp = vecSpherePos - c;
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

void Sphere::Bounce(XMVECTOR surfaceNormal)
{
	XMVECTOR sphereVel = XMLoadFloat3(&mSphereVel);
	XMVECTOR u = (XMVector4Dot(sphereVel, surfaceNormal)/ XMVector4Dot(surfaceNormal, surfaceNormal)) * surfaceNormal;
	XMVECTOR w = sphereVel - u;

	XMStoreFloat3(&mSphereVel, w - (RESTITUTION * u));
}

void Sphere::Draw()
{
	if (m_pSphereMesh)
	{
		m_pSphereMesh->Draw();
	}
}

