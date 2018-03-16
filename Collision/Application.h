#ifndef APPLICATION_H
#define APPLICATION_H

#define WIN32_LEAN_AND_MEAN

#include <assert.h>

#include <stdio.h>
#include <windows.h>
#include <d3d11.h>
#include <vector>
#include "Sphere.h"

#include "CommonApp.h"
#include "CommonMesh.h"
#

class HeightMap;
class Sphere;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// An example of creating a shader of your own, that fits in
// relatively neatly with the CommonApp functionality.
//
// Edit the shader as the program runs, then Alt+Tab back to it and
// press F5 to reload the shader. Instant feedback!
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define SPHERESIZE 50

__declspec(align(16)) struct SphereCollisionPair
{
	Sphere* firstSphere;
	Sphere* secondSphere;
	XMVECTOR normal;
	float penetration;

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

};


class Application:
public CommonApp
{
public:
	static Application* s_pApp;
protected:
	bool HandleStart();
	void HandleStop();
	void HandleUpdate(float dT);
	void HandleRender();
private:

	float m_frameCount;
	int m_currentHeightmap = 0;
	char* heightmaps[4] = {
		"Resources/heightmap0.bmp",
		"Resources/heightmap1.bmp",
		"Resources/heightmap2.bmp",
		"Resources/heightmap3.bmp"
	};

	bool m_reload;

	float m_rotationAngle;
	float m_cameraZ;
	bool m_bWireframe;

	int m_cameraState;

	HeightMap* m_pHeightMap;
	
	Sphere* sphereCollection[SPHERESIZE] = {nullptr};
	std::vector<SphereCollisionPair> collisionPairs;

	CommonMesh* m_pSphereMesh;
	void CreateSphere(XMFLOAT3 iSpherePos);
	bool SphereSphereIntersection(SphereCollisionPair& collisionPair);
	void BounceSpheres(SphereCollisionPair& collisionPair);
	void PositionalCorrection(SphereCollisionPair& collisionPair);
	void LoopSpheres();
	void CheckSphereCollisions();


	void ReloadShaders();
};

#endif