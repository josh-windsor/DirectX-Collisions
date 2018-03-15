#pragma once


//#include <DirectXMath.h>
//#include <malloc.h>
#include <vector>


#include "Sphere.h"


using namespace DirectX;


class Octree
{
public:
	Octree(XMVECTOR iBoundingBottomLeftFront, XMVECTOR iBoundingTopRightBack, int iDepth);
	~Octree();
	void AddNode(Sphere* iNode);
	void AddToChild(int subSection, XMVECTOR iBoundingBottomLeftFront, XMVECTOR iBoundingTopRightBack, Sphere* iNode);

	const int depthLimit = 10;

	Octree* children[8] = { nullptr };
	float sideLength;
	std::vector<Sphere*> nodes;
	int depth;
private:
	const XMVECTOR boundingBottomLeftFront;
	XMFLOAT3 centralPoint;
};

