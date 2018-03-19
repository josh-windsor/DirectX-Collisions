#pragma once


//#include <DirectXMath.h>
//#include <malloc.h>
#include <vector>


#include "Sphere.h"


using namespace DirectX;


__declspec(align(16)) class Octree
{
public:
	Octree(XMFLOAT3 iCenterPoint, float iSideLength);
	Octree();
	~Octree();

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}


	void AddNode(Octree* root, Sphere* iNode);
	void CleanTree(Octree* root);
	Sphere* sphereList = nullptr;
	Octree * children[8] = { nullptr };


private:



	XMFLOAT3 centralPoint;
	float sideLength;



};

