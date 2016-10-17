#pragma once


#include "ODXBuffer.h"
struct TDrawVertexType2
{
	float position[3];
	float normal[3];
	float texture[2];
};

class OBackGroundPlane
{
public:
	ODXBuffer m_VB;
	ODXBuffer m_IB;

	ID3D11Device*			m_pd3dDevice;
	ID3D11DeviceContext*	m_pImmediateContext;
	ID3D11InputLayout*      m_pVertexLayout;
	ID3D11VertexShader*     m_pVertexShader;
	ID3D11PixelShader*      m_pPixelShader;

	UINT		m_nVBCount;
	UINT		m_nIBCount;

	CTexture	m_Texture;

public:
	OBackGroundPlane(void);
	virtual ~OBackGroundPlane(void);

	bool Create(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext);


	bool SetLayout();
	bool SerVertexBuffer();
	bool SetIndexBuffer();
	bool Render();

};

