#include <d3d11.h>
#include "Texture.h"


#include "OBackGroundPlane.h"
#include "DXUT/Core/DXUT.h"
#include "DXUT/Optional/DXUTcamera.h"
#include "DXUT/Optional/SDKmisc.h"



OBackGroundPlane::OBackGroundPlane(void)
{
	m_pd3dDevice			= NULL;
	m_pImmediateContext		= NULL;
	m_pVertexLayout = nullptr;
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_nVBCount	= 0;
	m_nIBCount	= 0;
}


OBackGroundPlane::~OBackGroundPlane(void)
{
	m_VB.Release();
	m_IB.Release();

	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pPixelShader);
}

bool OBackGroundPlane::Create(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
	m_pd3dDevice = pd3dDevice;
	m_pImmediateContext = pImmediateContext;

	if( !SetLayout() )
	{
		return false;
	}

	if( !SerVertexBuffer())
	{
		return false;
	}

	if( !SetIndexBuffer() )
	{
		return false;
	}

	if( !m_Texture.Initialize(pd3dDevice, L"11.jpg"))
	{
		return false;
	}

	return true;
}

bool OBackGroundPlane::SetLayout()
{
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	DXUTCompileFromFile( L"Phong.fx", nullptr, "VSBG", "vs_4_0", dwShaderFlags, 0, &pVSBlob ) ;

	// Create the vertex shader
	hr = m_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader );
	if( FAILED( hr ) )
	{    
		SAFE_RELEASE( pVSBlob );
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = m_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
		return hr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob  = nullptr;
	V_RETURN( DXUTCompileFromFile( L"Phong.fx", nullptr, "PSBG", "ps_4_0", dwShaderFlags, 0, &pPSBlob  ) );

	// Create the pixel shader
	hr = m_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader );
	SAFE_RELEASE( pPSBlob );
	if( FAILED( hr ) )
		return hr;


	return true;
}

bool OBackGroundPlane::SerVertexBuffer()
{
	TDrawVertexType2 pVertex[4];

	pVertex[0].position[0] = -1.0f; pVertex[0].position[1] = 1.0f; pVertex[0].position[2] = 10.1;  
	pVertex[0].normal[0] = 0.0f; pVertex[0].normal[1] = 0.0f; pVertex[0].normal[2] = -1.0f;
	pVertex[0].texture[0] = 0.0f; pVertex[0].texture[1] = 0.0f;

	pVertex[1].position[0] = 1.0f; pVertex[1].position[1] = 1.0f; pVertex[1].position[2] = 10.1;  
	pVertex[1].normal[0] = 0.0f; pVertex[1].normal[1] = 0.0f; pVertex[1].normal[2] = -1.0f;
	pVertex[1].texture[0] = 1.0f; pVertex[1].texture[1] = 0.0f;

	pVertex[2].position[0] = 1.0f; pVertex[2].position[1] = -1.0f; pVertex[2].position[2] = 10.1;  
	pVertex[2].normal[0] = 0.0f; pVertex[2].normal[1] = 0.0f; pVertex[2].normal[2] = -1.0f;
	pVertex[2].texture[0] = 1.0f; pVertex[2].texture[1] = 1.0f;

	pVertex[3].position[0] = -1.0f; pVertex[3].position[1] = -1.0f; pVertex[3].position[2] = 10.1;  
	pVertex[3].normal[0] = 0.0f; pVertex[3].normal[1] = 0.0f; pVertex[3].normal[2] = -1.0f;
	pVertex[3].texture[0] = 0.0f; pVertex[3].texture[1] = 1.0f; 

	if(FAILED(m_VB.Create(m_pd3dDevice, sizeof(TDrawVertexType2),4,&pVertex, D3D11_BIND_VERTEX_BUFFER)) )
	{
		return false;
	}

	m_nVBCount = 4;
	return true;
}

bool OBackGroundPlane::SetIndexBuffer()
{
	DWORD indicies[] =
	{
		0,1,2,
		0,2,3,
	};

	if( FAILED(m_IB.Create( m_pd3dDevice, sizeof(DWORD), 6, &indicies,D3D11_BIND_INDEX_BUFFER)))
	{
		return false;
	}

	m_nIBCount = 6;

	return true;
}

bool OBackGroundPlane::Render()
{
	m_VB.Apply(m_pImmediateContext);
	m_IB.Apply(m_pImmediateContext);

	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
	
	m_pImmediateContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pImmediateContext->PSSetShader( m_pPixelShader, nullptr, 0 );

	ID3D11ShaderResourceView* pSRV = m_Texture.GetShaderResourceView();
	m_pImmediateContext->PSSetShaderResources( 2,1, &pSRV );


	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImmediateContext->DrawIndexed(6,0,0);

	return true;
}