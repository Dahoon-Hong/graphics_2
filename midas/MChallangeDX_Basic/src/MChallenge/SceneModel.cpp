#include "SceneModel.h"


CSceneModel::CSceneModel()
{
	m_pRasterState = nullptr;
	m_fResizeFactor = 1.f;

}
CSceneModel::~CSceneModel()
{
}
void CSceneModel::SetDevice( ID3D11Device* device, const std::string strPath )
{
	m_pDevice = device;
	m_strPath = strPath;
}

bool CSceneModel::ImportModel( )
{
	if ( m_strPath.empty() )
	{
		puts( "Path is invalid.\n" );
		return false;
	}

	// 기존 로드 데이터를 비움
	m_Importer.FreeScene();

	// 로드시 Mesh 데이터 생성 옵션 : Texture coordinate, Normal, Tangent & Bitangent를 생성해줌
	unsigned int qualityFlag = aiProcessPreset_TargetRealtime_MaxQuality;   // 속도가 중요하면 다음 옵션 사용 : aiProcessPreset_TargetRealtime_Fast 또는 aiProcessPreset_TargetRealtime_Quality
	// DirectX 용 Mesh 데이터 생성
	qualityFlag |= aiProcess_ConvertToLeftHanded;

	// 모델 로드 및 Mesh 데이터 생성
	m_aiScene = m_Importer.ReadFile( m_strPath, qualityFlag );
	if ( m_aiScene == nullptr )
	{
		puts( "File import failure.\n" );
		return false;
	}

	// 렌더링 모델 생성
	return true;
}

bool CSceneModel::MakeVBOInfo()
{
	ReleaseBuffer();
	assert( m_aiScene );

	// Mesh 데이터가 있는지 확인
	if ( m_aiScene->HasMeshes() == false )
	{
		puts( "No mesh data.\n" );
		return false;
	}
	// RasterizeState
	MakeRasterState();
	
	// AABB 계산 
	MakeAABB( m_aiScene->mRootNode );
	m_tAABB.CalCenter();

	float fRangeX = m_tAABB.arMax[0] - m_tAABB.arMin[0];
	float fRangeY = m_tAABB.arMax[1] - m_tAABB.arMin[1];
	float fRangeZ = m_tAABB.arMax[2] - m_tAABB.arMin[2];
	m_fResizeFactor = MODEL_RANGE / max(max( fRangeX, fRangeY ), fRangeZ);

	m_tAABB.arMax[0] = ( m_tAABB.arMax[0] - m_tAABB.arCen[0] ) * m_fResizeFactor;
	m_tAABB.arMax[1] = ( m_tAABB.arMax[1] - m_tAABB.arCen[1] ) * m_fResizeFactor;
	m_tAABB.arMax[2] = ( m_tAABB.arMax[2] - m_tAABB.arCen[2] ) * m_fResizeFactor;

	
	m_tAABB.arMin[0] = ( m_tAABB.arMin[0] - m_tAABB.arCen[0] ) * m_fResizeFactor;
	m_tAABB.arMin[1] = ( m_tAABB.arMin[1] - m_tAABB.arCen[1] ) * m_fResizeFactor;
	m_tAABB.arMin[2] = ( m_tAABB.arMin[2] - m_tAABB.arCen[2] ) * m_fResizeFactor;


	GenerateModelRecursive( m_aiScene->mRootNode );

	return true;
}
bool CSceneModel::MakeRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = TRUE;
	rasterDesc.MultisampleEnable = TRUE;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	
	HRESULT state = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterState);
	if(FAILED(state))
		return false;

	D3D11_BLEND_DESC blendStateDesc; 
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;        
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	state = m_pDevice->CreateBlendState(&blendStateDesc, &m_pBlendState);
	if(FAILED(state))
		return false;

	return true;
}
bool CSceneModel::MakeVBOBuffer( int VertexCount, int IdxCount, TDrawVertexType * arVertex, unsigned int* arIdx, TVBOInfo* pTVBO)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	HRESULT result;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(TDrawVertexType) * VertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = arVertex;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &pTVBO->VertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * IdxCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = arIdx;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pTVBO->IndexBuffer);
	if(FAILED(result))
	{
		return false;
	}
	pTVBO->nIdxCount = IdxCount;
	pTVBO->nVerCount = VertexCount;
	return true;
}

bool CSceneModel::MakeAABB( const aiNode * pNode )
{
	
	assert( pNode );
	// 모델 파일의 디렉터리 경로
	const std::string loaddir = m_strPath.substr( 0, m_strPath.find_last_of( "/\\" ) );
	// 현재 Node 모델의 Transformation 행렬
	const aiMatrix4x4 matTrans = pNode->mTransformation;

	for ( unsigned int idx = 0 ; idx < pNode->mNumMeshes ; idx++ )
	{
		// 현재 Node 모델의 Mesh 정보 Index
		const unsigned int meshIndex = pNode->mMeshes[idx];
		// 현재 Node 모델의 Mesh 정보
		const aiMesh* pMesh = m_aiScene->mMeshes[meshIndex];

		// Position, Normal, Texture coordinate, Tangent & Bitangent, Face 정보가 있는지 체크
		assert( pMesh->HasPositions() );

		for ( unsigned int fdx = 0 ; fdx < pMesh->mNumFaces ; fdx++ )
		{
			const aiFace face = pMesh->mFaces[fdx];

			// Face의 각 정점에 대해
			for ( unsigned int faceIndex = 0 ; faceIndex < face.mNumIndices ; faceIndex++ )
			{
				//printf( "Face : %d, Index : %d", fdx, faceIndex );
				const unsigned int dataIndex = face.mIndices[faceIndex];
				const aiVector3D position = pMesh->mVertices[dataIndex];
				float Pos[3];
				Pos[0] = -position.x;
				Pos[1] = position.y;
				Pos[2] = position.z;
				m_tAABB.Update( Pos );
			}
		}
	}
	bool isChildValid = false;
	if ( pNode->mNumChildren > 0 )
	{
		// Tree 형태로 구성된 자식 모델이 있는 경우 재귀적으로 모델 생성
		for ( unsigned int idx = 0 ; idx < pNode->mNumChildren ; idx++ )
		{
			isChildValid |= this->MakeAABB( pNode->mChildren[idx] );
		}
	}
	else
		isChildValid = true;
	return isChildValid;
}
bool CSceneModel::GenerateModelRecursive( const aiNode* pNode )
{
	assert( pNode );

	// 모델 파일의 디렉터리 경로
	const std::string loaddir = m_strPath.substr( 0, m_strPath.find_last_of( "/\\" ) );

	// 현재 Node 모델의 Transformation 행렬
	const aiMatrix4x4 matTrans = pNode->mTransformation;

	for ( unsigned int idx = 0 ; idx < pNode->mNumMeshes ; idx++ )
	{
		TMaterialInfo* pTMaterial = new TMaterialInfo(); TVBOInfo* pTVBO = new TVBOInfo();
		// 현재 Node 모델의 Mesh 정보 Index
		const unsigned int meshIndex = pNode->mMeshes[idx];
		// 현재 Node 모델의 Mesh 정보
		const aiMesh* pMesh = m_aiScene->mMeshes[meshIndex];

		// Position, Normal, Texture coordinate, Tangent & Bitangent, Face 정보가 있는지 체크
		assert( pMesh->HasPositions() );
		assert( pMesh->HasNormals() );
		assert( pMesh->HasTextureCoords( 0 ) );
		assert( pMesh->HasTangentsAndBitangents() );
		assert( pMesh->HasFaces() );

		// 현재 Mesh의 Material 정보 Index
		const unsigned int materialIndex = pMesh->mMaterialIndex;
		// 현재 Mesh의 Material 정보
		const aiMaterial* pMaterial = m_aiScene->mMaterials[materialIndex];

		// Material 투명도
		unsigned int opacityMax = 1;
		float opacity = 1.0f;
		bool hasOpacityValue = false;
		if ( aiGetMaterialFloatArray( pMaterial, AI_MATKEY_OPACITY, &opacity, &opacityMax ) == AI_SUCCESS )
		{
			// Material에 정의된 투명도 값이 있음
			hasOpacityValue = true;

			pTMaterial->Opacity = opacity;

			// TODO : 렌더링 Material 투명도 데이터 생성
		}

		// Material Ambient
		aiColor4D ambientColor;
		bool hasAmbientColor = false;
		if ( aiGetMaterialColor( pMaterial, AI_MATKEY_COLOR_AMBIENT, &ambientColor ) == AI_SUCCESS )
		{
			// Material에 정의된 Ambient 값이 있음
			hasAmbientColor = true;

			pTMaterial->Ambient.x = ambientColor.r; 
			pTMaterial->Ambient.y = ambientColor.g;
			pTMaterial->Ambient.z = ambientColor.b;
			pTMaterial->Ambient.w = ambientColor.a;
		}

		// Material Diffuse
		aiColor4D diffuseColor;
		bool hasDiffuseColor = false;
		if ( aiGetMaterialColor( pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor ) == AI_SUCCESS )
		{
			// Material에 정의된 Diffuse 값이 있음
			hasDiffuseColor = true;

			pTMaterial->Diffuse.x = diffuseColor.r; 
			pTMaterial->Diffuse.y = diffuseColor.g;
			pTMaterial->Diffuse.z = diffuseColor.b;
			pTMaterial->Diffuse.w = diffuseColor.a;
		}

		// Material Specular
		aiColor4D specularColor;
		bool hasSpecularColor = false;
		if ( aiGetMaterialColor( pMaterial, AI_MATKEY_COLOR_SPECULAR, &specularColor ) == AI_SUCCESS )
		{
			// Material에 정의된 Diffuse 값이 있음
			hasSpecularColor = true;

			pTMaterial->Specular.x = specularColor.r; 
			pTMaterial->Specular.y = specularColor.g;
			pTMaterial->Specular.z = specularColor.b;
			pTMaterial->Specular.w = specularColor.a;
		}

		RGB(255,0,0);

		// Material Shininess
		unsigned int shininessmax = 1;
		float shininess = 0.0f;
		bool hasShininess = false;
		if ( aiGetMaterialFloatArray( pMaterial, AI_MATKEY_SHININESS, &shininess, &shininessmax ) == AI_SUCCESS )
		{
			shininessmax = 1;
			float strength = 1.0f;
			if ( aiGetMaterialFloatArray( pMaterial, AI_MATKEY_SHININESS_STRENGTH, &strength, &shininessmax ) == AI_SUCCESS )
			{
				shininess *= strength;
			}
			// Material에 정의된 Shininess 값이 있음
			hasShininess = true;
			pTMaterial->Shininess = shininess;
		}

		// Texture Ambient
		cv::Mat ambientTexture;
		const unsigned int ambientTextureCount = pMaterial->GetTextureCount( aiTextureType_AMBIENT );
		bool hasAmbientTexture = false;
		if ( ambientTextureCount > 0 )
		{
			aiString path;
			// 제공되는 obj 모델에는 1개의 텍스쳐만 존재한다고 가정합니다. (첫번째 Texture만 사용)
			pMaterial->GetTexture( aiTextureType_AMBIENT, 0, &path );

			// Texture 로드
			const std::string fullpath = loaddir + "/" + std::string( path.C_Str() );   // 절대 경로를 사용함에 주의
			ambientTexture = cv::imread( fullpath.c_str(), cv::IMREAD_UNCHANGED );
			if ( ambientTexture.empty() == false )
			{
				// Texture가 성공적으로 로드된 경우
				hasAmbientTexture = true;
	
				pTMaterial->pAmbientTex = new CTexture();
				std::wstring unicode(fullpath.begin(), fullpath.end());
				pTMaterial->pAmbientTex->Initialize(m_pDevice, (WCHAR *)unicode.c_str() );
			}
		}

		// Texture Diffuse
		cv::Mat diffuseTexture;
		const unsigned int diffuseTextureCount = pMaterial->GetTextureCount( aiTextureType_DIFFUSE );
		bool hasDiffuseTexture = false;
		if ( diffuseTextureCount > 0 )
		{
			aiString path;
			// 제공되는 obj 모델에는 1개의 텍스쳐만 존재한다고 가정합니다. (첫번째 Texture만 사용)
			pMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &path );

			// Texture 로드
			const std::string fullpath = loaddir + "/" + std::string( path.C_Str() );   // 절대 경로를 사용함에 주의
			diffuseTexture = cv::imread( fullpath.c_str(), cv::IMREAD_UNCHANGED );
			if ( diffuseTexture.empty() == false )
			{
				// Texture가 성공적으로 로드된 경우
				hasDiffuseTexture = true;

				pTMaterial->pDiffuseTex = new CTexture();
				std::wstring unicode(fullpath.begin(), fullpath.end());
				pTMaterial->pDiffuseTex->Initialize(m_pDevice, (WCHAR *)unicode.c_str() );
			}
		}

		// Texture Specular
		cv::Mat specularTexture;
		const unsigned int specularTextureCount = pMaterial->GetTextureCount( aiTextureType_SPECULAR );
		bool hasSpecularTexture = false;
		if ( specularTextureCount > 0 )
		{
			aiString path;
			// 제공되는 obj 모델에는 1개의 텍스쳐만 존재한다고 가정합니다. (첫번째 Texture만 사용)
			pMaterial->GetTexture( aiTextureType_SPECULAR, 0, &path );

			// Texture 로드
			const std::string fullpath = loaddir + "/" + std::string( path.C_Str() );   // 절대 경로를 사용함에 주의
			specularTexture = cv::imread( fullpath.c_str(), cv::IMREAD_UNCHANGED );
			if ( specularTexture.empty() == false )
			{
				// Texture가 성공적으로 로드된 경우
				hasSpecularTexture = true;
				std::wstring unicode(fullpath.begin(), fullpath.end());
				pTMaterial->pSpecTex->Initialize(m_pDevice, (WCHAR *)unicode.c_str() );
			}
		}

		// Texture Alpha Map
		cv::Mat alphaTexture;
		const unsigned int alphaTextureCount = pMaterial->GetTextureCount( aiTextureType_OPACITY );
		bool hasAlphaTexture = false;
		if ( alphaTextureCount > 0 )
		{
			aiString path;
			// 제공되는 obj 모델에는 1개의 텍스쳐만 존재한다고 가정합니다. (첫번째 Texture만 사용)
			pMaterial->GetTexture( aiTextureType_OPACITY, 0, &path );

			// Texture 로드
			const std::string fullpath = loaddir + "/" + std::string( path.C_Str() );   // 절대 경로를 사용함에 주의
			alphaTexture = cv::imread( fullpath.c_str(), cv::IMREAD_UNCHANGED );
			if ( alphaTexture.empty() == false )
			{
				// Texture가 성공적으로 로드된 경우
				hasAlphaTexture = true;

				pTMaterial->pAlphaTex = new CTexture();
				std::wstring unicode(fullpath.begin(), fullpath.end());
				pTMaterial->pAlphaTex->Initialize(m_pDevice, (WCHAR *)unicode.c_str() );
			}
		}

		// Texture Normal Map
		cv::Mat normalTexture;
		const unsigned int normalTextureCount = pMaterial->GetTextureCount( aiTextureType_HEIGHT );     // obj 파일의 경우 map_bump/bump 에 대해 특별한 정의가 없어 Height로 Normal Map 정보가 들어오고 있음 (https://github.com/assimp/assimp/issues/430)
		bool hasNormalTexture = false;
		if ( normalTextureCount > 0 )
		{
			aiString path;
			// 제공되는 obj 모델에는 1개의 텍스쳐만 존재한다고 가정합니다. (첫번째 Texture만 사용)
			pMaterial->GetTexture( aiTextureType_HEIGHT, 0, &path );

			// Texture 로드
			const std::string fullpath = loaddir + "/" + std::string( path.C_Str() );   // 절대 경로를 사용함에 주의
			normalTexture = cv::imread( fullpath.c_str(), cv::IMREAD_UNCHANGED );
			if ( normalTexture.empty() == false )
			{
				// Texture가 성공적으로 로드된 경우
				hasNormalTexture = true;

				pTMaterial->pNormalTex = new CTexture();
				std::wstring unicode(fullpath.begin(), fullpath.end());
				pTMaterial->pNormalTex->Initialize(m_pDevice, (WCHAR *)unicode.c_str() );
			}
		}

		// Face 정보 파싱
		TDrawVertexType * arVertex = new TDrawVertexType[ pMesh->mNumVertices ]; // pos, nor, tex
		unsigned int * arFaceIdx = new unsigned int [ pMesh->mNumFaces * 3 ];
		for ( unsigned int fdx = 0 ; fdx < pMesh->mNumFaces ; fdx++ )
		{
			const aiFace face = pMesh->mFaces[fdx];

			// Face의 각 정점에 대해
			for ( unsigned int faceIndex = 0 ; faceIndex < face.mNumIndices ; faceIndex++ )
			{
				const unsigned int dataIndex = face.mIndices[faceIndex];
				arFaceIdx[ fdx * 3 + faceIndex ] = dataIndex;

				const aiVector3D position = pMesh->mVertices[dataIndex];
				arVertex[dataIndex].position[0] = ( position.x - m_tAABB.arCen[0] ) * m_fResizeFactor;
				arVertex[dataIndex].position[1] = ( position.y - m_tAABB.arCen[1] ) * m_fResizeFactor;
				arVertex[dataIndex].position[2] = ( position.z - m_tAABB.arCen[2] ) * m_fResizeFactor;
				
				m_tAABB.Update( arVertex[dataIndex].position );

				if ( pMesh->mColors[0] != nullptr )     // obj에서 Color는 1개만 존재하므로 첫번째 Color 데이터만 사용
				{
					// 정점에 Color 정보가 있는 경우
					const aiColor4D color = pMesh->mColors[0][dataIndex];


				}

				if ( pMesh->mNormals != nullptr )
				{
					// 정점에 Normal 정보가 있는 경우
					const aiVector3D normal = pMesh->mNormals[dataIndex];
					arVertex[dataIndex].normal[0] = normal.x;
					arVertex[dataIndex].normal[1] = normal.y;
					arVertex[dataIndex].normal[2] = normal.z;
				}

				if ( pMesh->mTextureCoords[0] != nullptr )  // obj에서 Texture Coordinate는 1개만 존재하므로 첫번째 Texture Coordinate 데이터만 사용
				{
					// 정점에 Texture coordinate 정보가 있는 경우
					const aiVector3D texcoord = pMesh->mTextureCoords[0][dataIndex];

					arVertex[dataIndex].texture[0] = texcoord.x;
					arVertex[dataIndex].texture[1] = texcoord.y;

				}

				if ( pMesh->mTangents != nullptr )
				{
					// 정점에 Tangent 정보가 있는 경우
					const aiVector3D tangent = pMesh->mTangents[dataIndex];

					arVertex[dataIndex].tangent[0] = tangent.x;
					arVertex[dataIndex].tangent[1] = tangent.y;
					arVertex[dataIndex].tangent[2] = tangent.z;

				}

				if ( pMesh->mBitangents != nullptr )
				{
					// 정점에 Bitangent 정보가 있는 경우
					const aiVector3D bitangent = pMesh->mBitangents[dataIndex];

					arVertex[dataIndex].bitangent[0] = bitangent.x;
					arVertex[dataIndex].bitangent[1] = bitangent.y;
					arVertex[dataIndex].bitangent[2] = bitangent.z;
				}
			}
		}
		if( arVertex != nullptr && arFaceIdx != nullptr )
		{
			MakeVBOBuffer( pMesh->mNumVertices, pMesh->mNumFaces * 3, arVertex, arFaceIdx, pTVBO);

			if( pTMaterial->Opacity < 1.f )
				m_vMeshVBOTrans.push_back( pTVBO );
			else
				m_vMeshVBOOpaque.push_back( pTVBO );

			m_vMaterial.push_back( pTMaterial );

			m_mapMeshMaterial[pTVBO] = pTMaterial;

			SAFE_DELETE( arVertex );
			SAFE_DELETE( arFaceIdx );
		}
	}

	bool isChildValid = false;
	if ( pNode->mNumChildren > 0 )
	{
		// Tree 형태로 구성된 자식 모델이 있는 경우 재귀적으로 모델 생성
		for ( unsigned int idx = 0 ; idx < pNode->mNumChildren ; idx++ )
		{
			isChildValid |= this->GenerateModelRecursive( pNode->mChildren[idx] );
		}
	}
	else
	{
		isChildValid = true;
	}
	return isChildValid;
}

void CSceneModel::ReleaseBuffer()
{
	for( int i = 0; i < m_vMeshVBOOpaque.size(); i++ )
	{
		TVBOInfo* tVBO = m_vMeshVBOOpaque[i];
		if(tVBO->IndexBuffer)
		{
			tVBO->IndexBuffer->Release();
			tVBO->IndexBuffer = nullptr;
		}
		if(tVBO->VertexBuffer)
		{
			tVBO->VertexBuffer->Release();
			tVBO->VertexBuffer = nullptr;
		}
		SAFE_DELETE( tVBO );
	}
	m_vMeshVBOOpaque.clear();
	for( int i = 0; i < m_vMeshVBOTrans.size(); i++ )
	{
		TVBOInfo* tVBO = m_vMeshVBOTrans[i];
		if(tVBO->IndexBuffer)
		{
			tVBO->IndexBuffer->Release();
			tVBO->IndexBuffer = nullptr;
		}
		if(tVBO->VertexBuffer)
		{
			tVBO->VertexBuffer->Release();
			tVBO->VertexBuffer = nullptr;
		}
		SAFE_DELETE( tVBO );
	}
	m_vMeshVBOTrans.clear();
	for( int i = 0; i < m_vMaterial.size(); i++ )
	{
		TMaterialInfo* ptMeterial = m_vMaterial[i];
		if(ptMeterial->pAmbientTex)
		{
			ptMeterial->pAmbientTex->Release();
			ptMeterial->pAmbientTex = nullptr;
		}
		if(ptMeterial->pDiffuseTex)
		{
			ptMeterial->pDiffuseTex->Release();
			ptMeterial->pDiffuseTex = nullptr;
		}
		if(ptMeterial->pSpecTex)
		{
			ptMeterial->pSpecTex->Release();
			ptMeterial->pSpecTex = nullptr;
		}
		if(ptMeterial->pAlphaTex)
		{
			ptMeterial->pAlphaTex->Release();
			ptMeterial->pAlphaTex = nullptr;
		}
		if(ptMeterial->pNormalTex)
		{
			ptMeterial->pNormalTex->Release();
			ptMeterial->pNormalTex = nullptr;
		}
		SAFE_DELETE( ptMeterial );
	}
	m_vMaterial.clear();

	m_mapMeshMaterial.clear();

	if( m_pRasterState != nullptr)
	{
		m_pRasterState->Release();
		m_pRasterState = nullptr;
	}
	if( m_pBlendState != nullptr)
	{
		m_pBlendState->Release();
		m_pBlendState = nullptr;
	}
}



