#ifndef APPLICATION_H
#define APPLICATION_H

#define WIN32_LEAN_AND_MEAN

#include <assert.h>

#include <stdio.h>
#include <windows.h>
#include <d3d11.h>
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

#define SPHERESIZE 15

class Application:
public CommonApp
{
public:
	static Application* s_pApp;
protected:
	bool HandleStart();
	void HandleStop();
	void HandleUpdate();
	void HandleRender();
private:

	float m_frameCount;

	bool m_reload;

	float m_rotationAngle;
	float m_cameraZ;
	bool m_bWireframe;

	int m_cameraState;

	HeightMap* m_pHeightMap;
	
	Sphere* sphereCollection[SPHERESIZE] = {nullptr};
	CommonMesh* m_pSphereMesh;
	void CreateSphere(XMFLOAT3 iSpherePos);
	bool SphereSphereIntersection(Sphere* sphere1, Sphere* sphere2, XMVECTOR& colN, float& distance);
	void PositionalCorrection(Sphere* sphere1, Sphere* sphere2, float penetration, XMVECTOR& colNormN);
	void LoopSpheres();

	void ReloadShaders();
};

#endif