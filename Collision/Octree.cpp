#include "Octree.h"



Octree::Octree(XMFLOAT3 iCenterPoint, float iSideLength)
{
	
	centralPoint = iCenterPoint;
	sideLength = iSideLength;
}


Octree::~Octree()
{
}

Octree::Octree() {}

Octree* Octree::BuildSubNode(XMFLOAT3 iCenterPoint, float iSideLength, int iMaxDepth)
{
	if (iMaxDepth >= 0)
	{
		Octree* newNode = new Octree(iCenterPoint, iSideLength);

		XMFLOAT3 offset;
		float step = iSideLength * 0.5f;

		for (int i = 0; i < 8; ++i)
		{
			offset.x = ((i & 1)) ? step : -step;
			offset.y = ((i & 2)) ? step : -step;
			offset.z = ((i & 4)) ? step : -step;

			newNode->children[i] = BuildSubNode(iCenterPoint + offset, step, iMaxDepth- 1);
		}
		return newNode;
	}
	return nullptr;
}

void Octree::AddNode(Octree* root, Sphere* iNode) 
{
	int index = 0;
	bool straddle = false;


	float objectPosition[3];
	float rootPosition[3];

	XMFLOAT3 f_objectPosition;
	XMStoreFloat3(&f_objectPosition, iNode->mSpherePos);

	XMFLOAT3 f_rootPosition = root->centralPoint;

	
	memcpy_s(objectPosition, sizeof(objectPosition), &f_objectPosition, sizeof(XMFLOAT3));
	memcpy_s(rootPosition  , sizeof(rootPosition  ), &f_rootPosition  , sizeof(XMFLOAT3));


	for (int i = 0; i < 3; ++i)
	{
		float d = objectPosition[i] - rootPosition[i];
		if (fabs(d) <= iNode->mRadius)
		{
			straddle = true;
			break;
		}

		if (d > 0.0f)
		{
			index |= (1 << i);
		}

	}
	if (!straddle && root->children[index])
	{
		AddNode(root->children[index], iNode);
	}
	else
	{
		iNode->mNextObj = root->sphereList;
		root->sphereList = iNode;
	}

}

void Octree::CleanTree(Octree* root)
{
	if (root)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (root->children[i] != nullptr)
			{
				CleanTree(root->children[i]);
				delete root->children[i];
			}
		}
	}
}

