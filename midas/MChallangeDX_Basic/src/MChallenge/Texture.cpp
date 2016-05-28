#include "Texture.h"
#include <d3d11.h>
#include <D3DX11tex.h>

CTexture::CTexture()
{
	m_pShaderResourceView = nullptr;
}

CTexture::~CTexture()
{
}

bool CTexture::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	// Load the texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_pShaderResourceView, NULL);
	if(FAILED(result))
	{
		return false;
	}
	return true;
}
void CTexture::Release()
{
	// Release the texture resource.
	if(m_pShaderResourceView)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}
	return;
}

ID3D11ShaderResourceView* CTexture::GetTexture()
{
	return m_pShaderResourceView;
}

bool CTexture::BindTexture( ID3D11DeviceContext* deviceContext , int SlotNum )
{
	if( m_pShaderResourceView != nullptr )
	deviceContext->PSSetShaderResources( SlotNum ,1, &(m_pShaderResourceView ));

	return true;
}
