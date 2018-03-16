#pragma once


//#include <DirectXMath.h>
//#include <malloc.h>
#include <vector>


#include "Sphere.h"


using namespace DirectX;


class Octree
{
public:
	Octree(XMFLOAT3 iCenterPoint, float iSideLength);
	~Octree();

	Octree* BuildSubNode(XMFLOAT3 iCenterPoint, float iSideLength, int iDepth);
	void AddNode(Octree* root, Sphere* iNode);
	void CleanTree(Octree* root);


private:

	Octree * children[8] = { nullptr };

	Sphere* sphereList;

	XMFLOAT3 centralPoint;
	float sideLength;

	static const int depthLimit = 5;


};

