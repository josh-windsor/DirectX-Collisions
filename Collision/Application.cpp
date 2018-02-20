#include "Application.h"
#include "HeightMap.h"


Application* Application::s_pApp = NULL;

const int CAMERA_TOP = 0;
const int CAMERA_ROTATE = 1;
const int CAMERA_MAX = 2;


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	s_pApp = this;

	m_frameCount = 0.0f;

	m_bWireframe = true;
	m_pHeightMap = new HeightMap( "Resources/heightmap.bmp", 2.0f, 0.75f );

	m_pSphereMesh = CommonMesh::NewSphereMesh(this, 1.0f, 16, 16);

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;

	m_reload = false;

	ReloadShaders();

	if (!this->CommonApp::HandleStart())
		return false;

	this->SetRasterizerState( false, m_bWireframe );

	m_cameraState = CAMERA_ROTATE;




	return true;
}

void Application::CreateSphere(XMFLOAT3 iSpherePos)
{
	Sphere * newSphere = new Sphere(iSpherePos, XMFLOAT3(0.0f, 0.2f, 0.0f), m_pSphereMesh, m_pHeightMap);
	static int lastIndex = 0;
	for (int i = 0; i < SPHERESIZE; i++)
	{

		if (sphereCollection[i] == nullptr)
		{
			sphereCollection[i] = newSphere;
			return;
		}
	}
	if (lastIndex == SPHERESIZE)
	{
		lastIndex = 0;
	}
	delete(sphereCollection[lastIndex]);
	sphereCollection[lastIndex] = newSphere;
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

void Application::HandleUpdate()
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

	static const float dT = 0.016f;
	static float timer = 0.0f;
	timer += 0.016f;
	bool over1 = false;
	if (timer >= 1.0f)
	{
		timer = 0.0f;
		over1 = true;
	}
	if ((this->IsKeyPressed(' ') && over1) || !this->IsKeyPressed(' '))
	{
		LoopSpheres();
		for (Sphere* sphere : sphereCollection)
		{
			if (sphere != nullptr)
			{
				sphere->CheckCollision();

				sphere->Update(dT);

			}
		}

	}

	
}

void Application::LoopSpheres() 
{
	for (int i = 0; i < SPHERESIZE; i++)
	{
		if (sphereCollection[i] != nullptr)
		{
			Sphere* sphere1 = sphereCollection[i];

			for (int j = i + 1; j < SPHERESIZE; j++)
			{
				if (sphereCollection[j] != nullptr && sphereCollection[i] != sphereCollection[j])
				{
					Sphere* sphere2 = sphereCollection[j];
					XMVECTOR colNorm;
					float distance;
					if (SphereSphereIntersection(sphere1, sphere2, colNorm, distance))
					{
						PositionalCorrection(sphere1, sphere2, 1 - distance, colNorm / sqrtf(distance));

						sphere1->Bounce(colNorm);
						sphere2->Bounce(-colNorm);

					}

				}
			}
		}
	}
}

bool Application::SphereSphereIntersection(Sphere* sphere1, Sphere* sphere2, XMVECTOR& colN, float& distance) 
{
	XMVECTOR vecSphere1Pos = XMLoadFloat3(&sphere1->mSpherePos);
	XMVECTOR vecSphere2Pos = XMLoadFloat3(&sphere2->mSpherePos);
	colN = vecSphere2Pos - vecSphere1Pos;

	distance = XMVectorGetX(XMVector3LengthSq(colN));

	float radius = sphere1->mRadius + sphere2->mRadius;
	radius = radius * radius;

	if (distance < radius)
	{
		return true;
	}

	return false;

}

void Application::PositionalCorrection(Sphere* sphere1, Sphere* sphere2, float penetration, XMVECTOR& colNormN)
{
	const float percent = 0.2; // usually 20% to 80%
	const float slop = 0.05; // usually 0.01 to 0.1
	XMVECTOR vecCorrection = max(penetration - slop, 0.0f) / 2 * penetration * colNormN;
	XMVECTOR vecSpherePos = XMLoadFloat3(&sphere1->mSpherePos);
	XMVECTOR vecSpherePos2 = XMLoadFloat3(&sphere2->mSpherePos);

	XMVECTOR vecCorrected = vecSpherePos - vecCorrection;
	XMVECTOR vecCorrected2 = vecSpherePos2 + vecCorrection;

	XMFLOAT3 correctionF, correctionF2;
	XMStoreFloat3(&correctionF, vecCorrected);
	XMStoreFloat3(&correctionF2, vecCorrected2);

	sphere1->mSpherePos = correctionF;
	sphere2->mSpherePos = correctionF2;
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
		if (sphere != nullptr)
		{

			XMMATRIX worldMtx;

			worldMtx = XMMatrixTranslation(sphere->mSpherePos.x, sphere->mSpherePos.y, sphere->mSpherePos.z);

			this->SetWorldMatrix(worldMtx);
			SetDepthStencilState(false, false);

			sphere->Draw();


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
