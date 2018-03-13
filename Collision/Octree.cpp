#include "Octree.h"



Octree::Octree(XMVECTOR iBoundingBottomLeftFront, XMVECTOR iBoundingTopRightBack, int iDepth) 
	:
	boundingBottomLeftFront(iBoundingBottomLeftFront),
	depth(iDepth)
{
	XMStoreFloat3(&centralPoint, (iBoundingBottomLeftFront + iBoundingTopRightBack) / 2);
	sideLength = XMVectorGetX(iBoundingTopRightBack) - XMVectorGetX(iBoundingBottomLeftFront);
	
}


Octree::~Octree()
{
}

void Octree::AddNode(Sphere* iNode) 
{
	if (depth == depthLimit)
	{
		nodes.push_back(iNode);
	}
	else
	{
		if (nodes.size() < 1)
		{
			nodes.push_back(iNode);
		}
		else
		{
			XMFLOAT3 nodePosition;
			XMStoreFloat3(&nodePosition, iNode->mSpherePos);
			if (nodePosition.x < centralPoint.x)
			{
				if (nodePosition.y < centralPoint.y)
				{
					if (nodePosition.z < centralPoint.z)
					{
						//return 0;
						AddToChild(0, boundingBottomLeftFront, XMLoadFloat3(&centralPoint), iNode);
					}
					else
					{
						//return 4;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(0, 0, sideLength / 2));
						AddToChild(4, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
				}
				else
				{
					if (nodePosition.z < centralPoint.z)
					{
						//return 2;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(0, sideLength / 2, 0));
						AddToChild(2, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
					else
					{
						//return 6;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(0, sideLength / 2, sideLength / 2));
						AddToChild(6, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
				}
			}
			else
			{
				if (nodePosition.y < centralPoint.y)
				{
					if (nodePosition.z < centralPoint.z)
					{
						//return 1;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(sideLength / 2, 0, 0));
						AddToChild(1, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
					else
					{
						//return 5;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(sideLength / 2, 0, sideLength / 2));
						AddToChild(5, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
				}
				else
				{
					if (nodePosition.z < centralPoint.z)
					{
						//return 3;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(sideLength / 2, sideLength / 2, 0));
						AddToChild(3, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
					else
					{
						//return 7;
						XMVECTOR correctionVec = XMLoadFloat3(&XMFLOAT3(sideLength / 2, sideLength / 2, sideLength / 2));
						AddToChild(7, boundingBottomLeftFront + correctionVec, XMLoadFloat3(&centralPoint) + correctionVec, iNode);
					}
				}
			}
			nodes.push_back(iNode);
			if (nodes.size() == 1)
			{
				AddNode(nodes[0]);
			}

		}
	}
}

void Octree::AddToChild(int subSection, XMVECTOR iBoundingBottomLeftFront, XMVECTOR iBoundingTopRightBack, Sphere* iNode)
{
	if (children[subSection] == nullptr)
	{
		children[subSection] = new Octree(iBoundingBottomLeftFront, iBoundingTopRightBack, depth + 1);
	}
	children[subSection]->AddNode(iNode);
}
