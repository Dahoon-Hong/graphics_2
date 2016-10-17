#include <d3d11.h>
#include <assert.h>

#include "ODxBuffer.h"


HRESULT ODXBuffer::Create(ID3D11Device* pDevice,
						  UINT iSize,
						  UINT iCounter,
						  void* pPoint,
						  UINT  BindFlags,
						  D3D11_USAGE Usage,
						  ID3D11Buffer** ppGetBuffer)
{
	assert(pPoint || iCounter > 0);
	HRESULT hr;
	m_iBindFlags = BindFlags;
	m_iSize = iSize;
	ID3D11Buffer**  ppBuffer = ppGetBuffer;

	if (ppBuffer == NULL)
	{
		ppBuffer = &m_pBuffer;
	}

	m_iCount = iCounter;

	m_BufferDesc.Usage = Usage;
	m_BufferDesc.ByteWidth = iSize * iCounter;
	m_BufferDesc.BindFlags = BindFlags;
	if (Usage == D3D11_USAGE_DYNAMIC)
		m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pPoint;


	hr = pDevice->CreateBuffer(&m_BufferDesc, &m_SubResourceData, ppBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
// 정점 버퍼는 0 번 슬롯을 사용한 1개 버퍼만 지원한다.
// 인덱스 버퍼는 32비트의 인덱스만 지원한다.
void ODXBuffer::Apply(ID3D11DeviceContext* pd3dDeviceContext)
{
	if (m_iBindFlags == D3D11_BIND_VERTEX_BUFFER)
	{
		UINT stride = m_iSize;
		UINT offset = 0;
		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pBuffer, &stride, &offset);
	}
	if (m_iBindFlags == D3D11_BIND_INDEX_BUFFER)
	{
		pd3dDeviceContext->IASetIndexBuffer(m_pBuffer, DXGI_FORMAT_R32_UINT, 0);
	}
}


UINT ODXBuffer::GetCount()
{
	return m_iCount;
}

bool ODXBuffer::Release()
{
	//SAFE_RELEASE(m_pBuffer);
	return true;
}
ODXBuffer::ODXBuffer(void)
{
	m_pBuffer = NULL;
	m_iCount = 0;
	m_iBindFlags = 0;
}

ODXBuffer::~ODXBuffer(void)
{
	ODXBuffer::Release();
}

