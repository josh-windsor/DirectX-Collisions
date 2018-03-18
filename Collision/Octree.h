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

XMFLOAT3 static operator+ (const XMFLOAT3 a, const XMFLOAT3 b)
{
	XMFLOAT3 output;
	output.x = a.x + b.x;
	output.y = a.y + b.y;
	output.z = a.z + b.z;
	return output;


}
