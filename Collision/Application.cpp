#include "Application.h"
#include "HeightMap.h"
#include "Octree.h"


Application* Application::s_pApp = NULL;

const int CAMERA_TOP = 0;
const int CAMERA_ROTATE = 1;
const int CAMERA_MAX = 2;



bool Application::HandleStart()
{
	s_pApp = this;

	m_frameCount = 0.0f;

	m_bWireframe = true;
	m_pHeightMap = new HeightMap( "Resources/heightmap0.bmp", 2.0f, 0.75f );

	m_pSphereMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;

	m_reload = false;

	ReloadShaders();

	if (!this->CommonApp::HandleStart())
		return false;

	this->SetRasterizerState( false, m_bWireframe );

	m_cameraState = CAMERA_ROTATE;

	//create all at beginning to save time
	for (int i = 0; i < SPHERESIZE; i++)
	{
		sphereCollection[i] = new Sphere(m_pSphereMesh, m_pHeightMap);
	}


	return true;
}

void Application::CreateSphere(XMFLOAT3 iSpherePos)
{
	static int lastIndex = 0;
	for (int i = 0; i < SPHERESIZE; i++)
	{

		if (!sphereCollection[i]->mSphereAlive)
		{
			
			sphereCollection[i]->StartSphere(XMLoadFloat3(&iSpherePos));
			return;
		}
	}
	if (lastIndex == SPHERESIZE)
	{
		lastIndex = 0;
	}
	delete(sphereCollection[lastIndex]);
	sphereCollection[lastIndex]->StartSphere(XMLoadFloat3(&iSpherePos));
	lastIndex++;


}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	delete m_pHeightMap;

	if( m_pSphereMesh )
		delete m_pSphereMesh;

	this->CommonApp::HandleStop();
}



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::ReloadShaders()
{
	if( m_pHeightMap->ReloadShader() == false )
		this->SetWindowTitle("Reload Failed - see Visual Studio output window. Press F5 to try again.");
	else
		this->SetWindowTitle("Collision: Zoom / Rotate Q, A / O, P, Camera C, Drop Sphere R, N and T, Wire W");
}

void Application::HandleUpdate(float dT)
{
	if( m_cameraState == CAMERA_ROTATE )
	{
		if (this->IsKeyPressed('Q') && m_cameraZ > 38.0f )
			m_cameraZ -= 1.0f;
		
		if (this->IsKeyPressed('A'))
			m_cameraZ += 1.0f;

		if (this->IsKeyPressed('O'))
			m_rotationAngle -= .01f;
		
		if (this->IsKeyPressed('P'))
			m_rotationAngle += .01f;
	}

	
	static bool dbC = false;

	if (this->IsKeyPressed('C') )	
	{
		if( !dbC )
		{
			if( ++m_cameraState == CAMERA_MAX )
				m_cameraState = CAMERA_TOP;

			dbC = true;
		}
	}
	else
	{
		dbC = false;
	}


	static bool dbW = false;
	if (this->IsKeyPressed('W') )	
	{
		if( !dbW )
		{
			m_bWireframe = !m_bWireframe;
			this->SetRasterizerState( false, m_bWireframe );
			dbW = true;
		}
	}
	else
	{
		dbW = false;
	}


	if (this->IsKeyPressed(VK_F5))
	{
		if (!m_reload)
		{
			ReloadShaders();
			m_reload = true;
		}
	}
	else
		m_reload = false;

	static bool dbR = false;
	if (this->IsKeyPressed('R') )
	{
		if( dbR == false )
		{
			static int dx = 0;
			static int dy = 0;
			XMFLOAT3 mSpherePos = XMFLOAT3((float)((rand() % 14 - 7.0f) - 0.5), 20.0f, (float)((rand() % 14 - 7.0f) - 0.5));

			CreateSphere(mSpherePos);
			dbR = true;
		}
	}
	else
	{
		dbR = false;
	}

	static bool dbT = false;
	if (this->IsKeyPressed('T'))
	{
		if (dbT == false)
		{
			static int dx = 0;
			static int dy = 0;
			XMFLOAT3 mSpherePos = XMFLOAT3(mSpherePos.x, 20.0f, mSpherePos.z);

			CreateSphere(mSpherePos);

			dbT = true;
		}
	}
	else
	{
		dbT = false;
	}

	static int dx = 0;
	static int dy = 0;
	static int seg = 0;
	static bool dbN = false;

	if (this->IsKeyPressed('N') )
	{
		if( dbN == false )
		{
			if( ++seg == 2 )
			{
				seg=0;
				if( ++dx==15 ) 
				{
					if( ++dy ==15 ) dy=0;
					dx=0;
				}
			}
			XMFLOAT3 mSpherePos;
			if( seg == 0 )
				mSpherePos = XMFLOAT3(((dx - 7.0f) * 2) - 0.5f, 20.0f, ((dy - 7.0f) * 2) - 0.5f);
			else
				mSpherePos = XMFLOAT3(((dx - 7.0f) * 2) + 0.5f, 20.0f, ((dy - 7.0f) * 2) + 0.5f);

			CreateSphere(mSpherePos);
			dbN = true;
		}
	}
	else
	{
		dbN = false;
	}



	static bool dbU = false;
	static int triIndex = 0;
	static int vertexIndex = 0;

	if (this->IsKeyPressed('U'))
	{
		if (dbU == false)
		{
			triIndex++;

			XMFLOAT3* triangle[5];
			m_pHeightMap->GetTriangle(triIndex, triangle);


			XMFLOAT3 mSpherePos = XMFLOAT3(triangle[4]->x, 20.0f, triangle[4]->z);



			vertexIndex = 0;
			CreateSphere(mSpherePos);

			dbU = true;
		}
		
	}
	else
	{
		dbU = false;
	}


	static bool dbI = false;
	if (this->IsKeyPressed('I'))
	{
		if (dbI == false)
		{

			triIndex--;

			XMFLOAT3* triangle[5];
			m_pHeightMap->GetTriangle(triIndex, triangle);


			XMFLOAT3 mSpherePos = XMFLOAT3(triangle[4]->x, 20.0f, triangle[4]->z);



			vertexIndex = 0;
			CreateSphere(mSpherePos);
			dbI = true;
		}

	}
	else
	{
		dbI = false;
	}

	static bool dbD = false;
	if (this->IsKeyPressed('D'))
	{
		if (dbD == false)
		{
			XMFLOAT3* triangle[5];
			m_pHeightMap->GetTriangle(triIndex, triangle);
			XMFLOAT3 mSpherePos;
			if (vertexIndex == 3)
			{
				mSpherePos = XMFLOAT3(triangle[4]->x, 20.0f, triangle[4]->z);
				vertexIndex = -1;
			}
			else
			{
				mSpherePos = XMFLOAT3(triangle[vertexIndex]->x, 20.0f, triangle[vertexIndex]->z);
			}


			vertexIndex++;

			CreateSphere(mSpherePos);
			dbD = true;

		}
	}
	else
	{
		dbD = false;
	}
	static bool dbF = false;
	if (this->IsKeyPressed('F'))
	{
		if (dbF == false)
		{
			XMFLOAT3 mSpherePos;

			mSpherePos = XMFLOAT3(5.0f, 20.0f, 0.0f);

			CreateSphere(mSpherePos);

			mSpherePos = XMFLOAT3(-5.0f, 20.0f, 0.0f);

			CreateSphere(mSpherePos);
			dbF = true;

		}
	}
	else
	{
		dbF = false;
	}

	static bool dbH = false;
	static bool hToggle = true;
	if (this->IsKeyPressed('H'))
	{
		if (dbH == false)
		{
			if (hToggle)
			{
				m_pHeightMap->DisableBelowLevel(4);
			}
			else
			{
				m_pHeightMap->EnableAll();
			}
			hToggle = !hToggle;
			dbH = true;

		}
	}
	else
	{
		dbH = false;
	}

	static float timer = 0.0f;
	timer += dT;
	bool over1 = false;
	if (timer >= 1.0f)
	{
		timer = 0.0f;
		over1 = true;
	}
	if ((this->IsKeyPressed(' ') && over1) || !this->IsKeyPressed(' '))
	{
		//LoopSpheres();
		for (Sphere* sphere : sphereCollection)
		{
			if (sphere->mSphereAlive)
			{

				sphere->Update(dT);
			}

		}
		LoopSpheres();
		CheckSphereCollisions();

	}

	static bool dbL = false;
	if (this->IsKeyPressed('L'))
	{
		if (dbL == false)
		{
			delete(m_pHeightMap);
			m_currentHeightmap++;
			if (m_currentHeightmap > 3)
			{
				m_currentHeightmap = 0;
			}
			m_pHeightMap = new HeightMap(heightmaps[m_currentHeightmap], 2.0f, 0.75f);
			m_pHeightMap->RebuildVertexData();
		}
	}
	else
	{
		dbL = false;
	}



	
}

//void Application::LoopSpheres()
//{
//	XMFLOAT3 boundingBottomLeftFront;
//	XMFLOAT3 boundingTopRightBack;
//	bool foundSphere = false;
//	bool firstLoop = true;
//	for (int i = 0; i < SPHERESIZE; i++)
//	{
//		if (sphereCollection[i]->mSphereAlive)
//		{
//			XMFLOAT3 currentSpherePos;
//			XMStoreFloat3(&currentSpherePos, sphereCollection[i]->mSpherePos);
//			if (firstLoop)
//			{
//				boundingBottomLeftFront = currentSpherePos;
//				firstLoop = false;
//			}
//
//			if (currentSpherePos.x < boundingBottomLeftFront.x)
//			{
//				boundingBottomLeftFront.x = currentSpherePos.x;
//			}
//			if (currentSpherePos.y < boundingBottomLeftFront.y)
//			{
//				boundingBottomLeftFront.y = currentSpherePos.y;
//			}
//			if (currentSpherePos.z < boundingBottomLeftFront.z)
//			{
//				boundingBottomLeftFront.z = currentSpherePos.z;
//			}
//			if (currentSpherePos.x > boundingTopRightBack.x)
//			{
//				boundingTopRightBack.x = currentSpherePos.x;
//			}
//			if (currentSpherePos.y > boundingTopRightBack.y)
//			{
//				boundingTopRightBack.y = currentSpherePos.y;
//			}
//			if (currentSpherePos.z > boundingTopRightBack.z)
//			{
//				boundingTopRightBack.z = currentSpherePos.z;
//			}
//			foundSphere = true;
//		}
//	}
//	int nodes = 0;
//	if (foundSphere)
//	{
//		Octree baseOct = Octree(XMLoadFloat3(&boundingBottomLeftFront), XMLoadFloat3(&boundingTopRightBack), 1);
//		for (int i = 0; i < SPHERESIZE; i++)
//		{
//			if (sphereCollection[i]->mSphereAlive)
//			{
//				baseOct.AddNode(sphereCollection[i]);
//				nodes++;
//			}
//			if (nodes > 3)
//			{
//
//				int testInt = 0;
//				testInt++;
//			}
//		}
//	}
//
//	
//}

void Application::LoopSpheres() 
{
	for (int i = 0; i < SPHERESIZE; ++i)
	{
		if (sphereCollection[i]->mSphereAlive)
		{
			Sphere* sphere1 = sphereCollection[i];

			for (int j = 1; j < SPHERESIZE; ++j)
			{
				if (sphereCollection[j]->mSphereAlive && sphereCollection[i] != sphereCollection[j])
				{
					SphereCollisionPair newPair;
					newPair.firstSphere = sphereCollection[i];
					newPair.secondSphere = sphereCollection[j];
					newPair.normal = XMVectorZero();
					newPair.penetration = 0.0f;
					collisionPairs.push_back(newPair);
				}
			}
		}
	}

}

void Application::CheckSphereCollisions() 
{
	for (SphereCollisionPair sCP : collisionPairs)
	{
		if (SphereSphereIntersection(sCP))

		{
			BounceSpheres(sCP);
			PositionalCorrection(sCP);
		}
	}
	collisionPairs.clear();
}

bool Application::SphereSphereIntersection(SphereCollisionPair& collisionPair)
{

	XMVECTOR collisionNormal = collisionPair.firstSphere->mSpherePos - collisionPair.secondSphere->mSpherePos;

	float distance = XMVectorGetX(XMVector3LengthSq(collisionNormal));

	float radius = collisionPair.firstSphere->mRadius + collisionPair.secondSphere->mRadius;
	float radiusSqr = radius * radius;

	if (distance > radiusSqr)
	{
		return false;
	}

	distance = sqrtf(distance);
	collisionPair.normal = collisionNormal / distance;
	collisionPair.penetration = radius - distance;

	return true;

}

void Application::BounceSpheres(SphereCollisionPair& collisionPair)
{
	float normalVelocityLength = XMVectorGetX(XMVector3Dot((collisionPair.secondSphere->mSphereVel - collisionPair.firstSphere->mSphereVel), collisionPair.normal));


	float u = (-1.5f * normalVelocityLength) / (collisionPair.firstSphere->mInvMass + collisionPair.secondSphere->mInvMass);


	XMVECTOR push = u * collisionPair.normal;

	collisionPair.firstSphere->mSphereVel -= push;
	collisionPair.secondSphere->mSphereVel += push;

}

void Application::PositionalCorrection(SphereCollisionPair& collisionPair)
{
	const float percent = 0.99f; // usually 20% to 80%
	const float slop = 0.001f; // usually 0.01 to 0.1
	XMVECTOR vecCorrection = max(collisionPair.penetration - slop, 0.0f) / (collisionPair.firstSphere->mInvMass + collisionPair.secondSphere->mInvMass) * percent * collisionPair.normal;

	collisionPair.firstSphere->mSpherePos  += vecCorrection;
	collisionPair.secondSphere->mSpherePos -= vecCorrection;
}




//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleRender()
{
	XMVECTOR vCamera, vLookat;
	XMVECTOR vUpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX matProj, matView;

	switch( m_cameraState )
	{
		case CAMERA_TOP:
			vCamera = XMVectorSet(0.0f, 100.0f, 0.1f, 0.0f);
			vLookat = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			matView = XMMatrixLookAtLH(vCamera, vLookat, vUpVector);
			matProj = XMMatrixOrthographicLH(64, 36, 1.5f, 5000.0f);
			break;
		case CAMERA_ROTATE:
			vCamera = XMVectorSet(sin(m_rotationAngle)*m_cameraZ, (m_cameraZ*m_cameraZ) / 50, cos(m_rotationAngle)*m_cameraZ, 0.0f);
			vLookat = XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f);
			matView = XMMatrixLookAtLH(vCamera, vLookat, vUpVector);
			matProj = XMMatrixPerspectiveFovLH(float(D3DX_PI / 7), 2, 1.5f, 5000.0f);
			break;
	}

	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.55f, 0.55f, 0.65f));
	this->EnableDirectionalLight(2, XMFLOAT3(1.f, -1.f, 1.f), XMFLOAT3(0.15f, 0.15f, 0.15f));

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->Clear(XMFLOAT4(0.05f, 0.05f, 0.5f, 1.f));

	SetDepthStencilState(false, true);
	m_pHeightMap->Draw(m_frameCount);

	for (Sphere* sphere : sphereCollection)
	{
		if (sphere->mSphereAlive)
		{

			XMMATRIX worldMtx;
			XMFLOAT3 fSpherePos;
			XMStoreFloat3(&fSpherePos, sphere->mSpherePos);
			worldMtx = XMMatrixTranslation(fSpherePos.x, fSpherePos.y, fSpherePos.z);

			this->SetWorldMatrix(worldMtx);
			SetDepthStencilState(true, true);
			sphere->Draw();
		}
	}

	m_frameCount++;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////



int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	Application application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
