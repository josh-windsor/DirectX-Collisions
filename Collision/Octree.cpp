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


void Octree::AddNode(Octree* root, Sphere* iNode) 
{
	int index = 0;
	bool straddle = false;


	float objectPosition[3];
	float rootPosition[3];

	XMFLOAT3 f_objectPosition;
	XMStoreFloat3(&f_objectPosition, iNode->mSpherePos);

	XMFLOAT3 f_rootPosition = root->centralPoint;

	//can copy as float3 is the same size as the array
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
		//insert into subtree

		AddNode(root->children[index], iNode);
	}
	else
	{
		//Stradling so has no child, will add onto list
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

