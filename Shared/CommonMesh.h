#ifndef HEADER_AEDE80DDE62645E6ACECBDDE88EBE6DA
#define HEADER_AEDE80DDE62645E6ACECBDDE88EBE6DA

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct ID3DXMesh;
struct ID3DXBuffer;

#include "CommonApp.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class CommonMesh
{
public:
	static CommonMesh *LoadFromXFile(CommonApp *pApp, const char *pFileName);
	static CommonMesh *NewBoxMesh(CommonApp *pApp, float width, float height, float depth);
	static CommonMesh *NewCylinderMesh(CommonApp *pApp, float radius1, float radius2, float length, unsigned slices, unsigned stacks);
	static CommonMesh *NewSphereMesh(CommonApp *pApp, float radius1, unsigned slices, unsigned stacks);
	static CommonMesh *NewTorusMesh(CommonApp *pApp, float innerRadius, float outerRadius, unsigned sides, unsigned rings);
	static CommonMesh *NewTeapotMesh(CommonApp *pApp);
		
	~CommonMesh();

	void Draw();

	// With care, shaders can be replaced.
	//
	// Remember that the vertex type and the shader are related by the input
	// layout...
	size_t GetNumSubsets() const;
	CommonApp::Shader *GetSubsetShader(size_t subsetIndex) const;
	void SetSubsetShader(size_t subsetIndex, CommonApp::Shader *pShader);
	void DrawSubset(size_t subsetIndex);
	void GetSubsetLocalAABB(size_t subsetIndex, D3DXVECTOR3 *pLocalAABBMin, D3DXVECTOR3 *pLocalAABBMax) const;

	// Many meshes have only one subset.
	void SetShaderForAllSubsets(CommonApp::Shader *pShader);
protected:
private:
	struct Subset;

	CommonMesh();

	Subset *m_pSubsets;
	size_t m_numSubsets;

	CommonApp *m_pApp;

	static CommonMesh *ConvertFromD3DXMesh(CommonApp *pApp, ID3DXMesh *pMesh9, ID3DXBuffer *pMaterialsBuffer9);

	CommonMesh(const CommonMesh &);
	CommonMesh &operator=(const CommonMesh &);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#endif//HEADER_AEDE80DDE62645E6ACECBDDE88EBE6DA
