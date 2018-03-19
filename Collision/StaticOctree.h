#pragma once


//#include <DirectXMath.h>
//#include <malloc.h>
#include <vector>

#include "HeightMap.h"


using namespace DirectX;

__declspec(align(16)) struct InputVertex
{
	InputVertex* nextVert = nullptr;
	int index;
	XMVECTOR centralPosition;
	float radius = 1.f;

	bool inuse = false;

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

};

__declspec(align(16)) class StaticOctree
{
public:
	StaticOctree(XMFLOAT3 iCenterPoint, float iSideLength);
	StaticOctree();
	~StaticOctree();

	void AddNode(StaticOctree* root, InputVertex* iNode);
	void CleanTree(StaticOctree* root);
	InputVertex* GetCollisions(StaticOctree* root, XMFLOAT3 f_spherePos, int iMaxDepth);

	InputVertex* vertList = nullptr;
	StaticOctree* children[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	XMFLOAT3 centralPoint;


private:


	bool ContainedInOct(XMFLOAT3 centerPoint, float sideLen, XMFLOAT3 f_spherePos);

	float sideLength;



};
