#include "StaticOctree.h"



StaticOctree::StaticOctree(XMFLOAT3 iCenterPoint, float iSideLength)
{

	centralPoint = iCenterPoint;
	sideLength = iSideLength;
}


StaticOctree::~StaticOctree()
{
}

StaticOctree::StaticOctree() {}


void StaticOctree::AddNode(StaticOctree* root, InputVertex* iNode)
{
	int index = 0;
	bool straddle = false;


	float objectPosition[3];
	float rootPosition[3];

	XMFLOAT3 f_objectPosition;
	XMStoreFloat3(&f_objectPosition, iNode->centralPosition);

	XMFLOAT3 f_rootPosition = root->centralPoint;

	//can copy as float3 is the same size as the array
	memcpy_s(objectPosition, sizeof(objectPosition), &f_objectPosition, sizeof(XMFLOAT3));
	memcpy_s(rootPosition, sizeof(rootPosition), &f_rootPosition, sizeof(XMFLOAT3));


	for (int i = 0; i < 3; ++i)
	{
		float d = objectPosition[i] - rootPosition[i];
		if (fabs(d) <= iNode->radius)
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
		iNode->nextVert = root->vertList;
		root->vertList = iNode;
	}

}

void StaticOctree::CleanTree(StaticOctree* root)
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


InputVertex* StaticOctree::GetCollisions(StaticOctree* root, XMFLOAT3 f_spherePos, int iMaxDepth)
{
	//if it is at the bottom depth
	if (iMaxDepth == 0)
	{
		//check if objects exist
		if (!root && root->vertList == nullptr)
		{
			return nullptr;
		}
		//return the bottom octs vert list
		return root->vertList;
	}
	//loop through all childre
	for (int i = 0; i < 8; i++)
	{
		if (root->children[i] != nullptr)
		{
			//if the sphere is contained within the oct
			if (ContainedInOct(root->centralPoint, root->sideLength, f_spherePos))
			{
				//call on its children till at bottom level
				GetCollisions(root->children[i], f_spherePos, iMaxDepth - 1);
				break;
			}
		}
	}
}

bool StaticOctree::ContainedInOct(XMFLOAT3 centerPoint,float sideLen, XMFLOAT3 f_spherePos)
{
	//check if the sphere is within the oct by checking against the center and bounds
	bool x = (centerPoint.x - sideLen < f_spherePos.x) && (centerPoint.x + sideLen > f_spherePos.x);
	bool y = (centerPoint.y - sideLen < f_spherePos.y) && (centerPoint.y + sideLen > f_spherePos.y);
	bool z = (centerPoint.z - sideLen < f_spherePos.z) && (centerPoint.z + sideLen > f_spherePos.z);

	return x && y && z;
}

