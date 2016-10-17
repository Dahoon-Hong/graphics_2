#pragma once
#pragma warning(disable:4819)

#include <iostream>
#include <string>
#include <cassert>

// obj 모델 로드 라이브러리
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// 이미지(텍스쳐) 로드 라이브러리
#include <opencv2/opencv.hpp>

#include <d3d11.h>
#include <DirectXMath.h>
#include "Texture.h"
#include "DataTypeDef.h"
#include "DXUT/Core/DXUT.h"
#define MODEL_RANGE 200.f

using namespace DirectX;

struct TDrawVertexType
{
	float position[3];
	float normal[3];
	float texture[2];
};

struct TMaterialInfo
{
	TMaterialInfo()
	{
		Emissive = XMFLOAT4(0.1f,0.1f,0.1f,1.f);
		Ambient = XMFLOAT4(0.1f,0.1f,0.1f,1.f);
		Diffuse = XMFLOAT4(0.1f,0.1f,0.1f,1.f);
		Specular = XMFLOAT4(0.1f,0.1f,0.1f,1.f);
		Shininess = 32.f;
		Opacity = 1.f;

		pAmbientTex = nullptr;
		pDiffuseTex = nullptr;
		pSpecTex = nullptr;
		pNormalTex = nullptr;
		pAlphaTex = nullptr;
	}
	~TMaterialInfo(){}

	XMFLOAT4  Emissive;      
	XMFLOAT4  Ambient;        
	XMFLOAT4  Diffuse;        
	XMFLOAT4  Specular;       
	float	  Shininess;
	float	  Opacity;

	CTexture* pAmbientTex;
	CTexture* pDiffuseTex;
	CTexture* pSpecTex;
	CTexture* pAlphaTex;
	CTexture* pNormalTex;
};



struct TVBOInfo
{
	ID3D11Buffer *VertexBuffer;
	ID3D11Buffer *IndexBuffer;

	UINT nVerCount;
	UINT nIdxCount;
};

class CSceneModel
{
public:
	CSceneModel();
	~CSceneModel();

	void SetDevice( ID3D11Device* device, const std::string strPath );

	bool ImportModel();
	bool MakeVBOInfo();
	void ReleaseBuffer();

	std::vector < TVBOInfo* >& GetMeshOpaqList() { return m_vMeshVBOOpaque; }
	std::vector < TVBOInfo* >& GetMeshTransList() { return m_vMeshVBOTrans; }
	std::map< TVBOInfo*, TMaterialInfo* > GetMeshMaterialMap() { return m_mapMeshMaterial; }

	ID3D11RasterizerState * GetRasterStateBuffer() { return m_pRasterState; }
	ID3D11BlendState * GetBlendStateBuffer() { return m_pBlendState; }
	TAABBInfo	GetAABB() { return m_tAABB; };
protected:
	bool GenerateModelRecursive( const aiNode* pNode );
	bool MakeVBOBuffer( int VertexCount, int IdxCount, TDrawVertexType * arVertex, unsigned int* arIdx, TVBOInfo* pTVBO );
	bool MakeRasterState();
	bool MakeAABB( const aiNode * pNode );

	ID3D11Device* m_pDevice;
	const aiScene * m_aiScene;	
	std::string m_strPath;

	std::vector < TVBOInfo* > m_vMeshVBOOpaque; // 불투명 재질
	std::vector < TVBOInfo* > m_vMeshVBOTrans;	// 투명 재질
	std::vector < TMaterialInfo* > m_vMaterial;

	std::map< TVBOInfo*, TMaterialInfo* > m_mapMeshMaterial;

	ID3D11RasterizerState*	m_pRasterState;
	ID3D11BlendState*		m_pBlendState;
	
	Assimp::Importer m_Importer;
	TAABBInfo	m_tAABB;
	float	m_fResizeFactor;
};