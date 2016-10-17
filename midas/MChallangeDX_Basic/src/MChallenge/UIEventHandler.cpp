#include <Windows.h>
#include "UIEventHandler.h"
#include <d3dx9math.h>
#include <math.h>
#include "umath.h"
#include <stdio.h>
#include "SceneModel.h"

CUIEventHandler::CUIEventHandler(void)
{
}


CUIEventHandler::~CUIEventHandler(void)
{
}

void CUIEventHandler::Initialize( int nWinXSize, int nWinYSize )
{
	m_bRotating = false;
	m_vWinSize.x = nWinXSize;
	m_vWinSize.y = nWinYSize;
	_vMax = D3DXVECTOR3(10,10,10);
	_vMin  = D3DXVECTOR3(0,0,0);
	m_vLookVector = XMFLOAT3(0,0,0);
	m_tCameraInit.vCenter = m_tCameraInfo.vCenter;
	m_tCameraInit.vPos = m_tCameraInfo.vPos;
	m_tCameraInit.vUp = m_tCameraInfo.vUp;
	m_tRotateInit.vRotate = m_tRotateInfo.vRotate;
	bSpin = false;
}
void CUIEventHandler::Reset(){
	bSpin = false;
	SetObjectView(m_tCameraInfo.max, m_tCameraInfo.min);
}
void CUIEventHandler::OnMouseEvent( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int iMouseX = ( short )LOWORD( lParam );
	int iMouseY = ( short )HIWORD( lParam );
	
	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
		{
			m_bRotating = true;
			break;
		}
		case WM_MOUSEMOVE:
		{
			OnMouseMotionEvent( iMouseX, iMouseY );
			break;
		}
		case WM_LBUTTONUP:
		{
			m_bRotating = false;
			break;
		}
		case WM_RBUTTONDOWN:
			m_bZoom = true;
			break;
		case WM_RBUTTONUP:
			m_bZoom = false;
			break;
		case WM_MBUTTONDOWN:
			m_bPanning = true;
			break;
		case WM_MBUTTONUP:
			m_bPanning = false;
			break;
	}
}

void CUIEventHandler::OnMouseMotionEvent( int nWinX, int nWinY )
{
	m_vPosOld = m_vPosNew;
	m_vPosNew.x = nWinX;
	m_vPosNew.y = nWinY;

	float fRel_x = (m_vPosNew.x - m_vPosOld.x) / (float)m_vWinSize.x;
	float fRel_y = (m_vPosNew.y - m_vPosOld.y) / (float)m_vWinSize.y;
	if (m_bRotating)
	{
		m_tRotateInfo.vRotate.y -= D3DXToRadian(fRel_x * 180);
		m_tRotateInfo.vRotate.x -= D3DXToRadian(fRel_y * 180);
	}

	else if(m_bPanning){
		float len = 2 *	sqrtf(pow((m_tCameraInfo.vPos.x  - m_tCameraInfo.vCenter.x),2.0f) +
								pow((m_tCameraInfo.vPos.y  - m_tCameraInfo.vCenter.y),2.0f) +
								pow((m_tCameraInfo.vPos.z  - m_tCameraInfo.vCenter.z),2.0f)) *
								tanf(m_tCameraInfo.fFOV/2);

		float _x = m_tCameraInfo.vUp.y * (m_tCameraInfo.vCenter.z -  m_tCameraInfo.vPos.z) - m_tCameraInfo.vUp.z * (m_tCameraInfo.vCenter.y - m_tCameraInfo.vPos.y);
		float _y =-( m_tCameraInfo.vUp.x * (m_tCameraInfo.vCenter.z -  m_tCameraInfo.vPos.z) - m_tCameraInfo.vUp.z * (m_tCameraInfo.vCenter.x - m_tCameraInfo.vPos.x));
		float _z = m_tCameraInfo.vUp.x * (m_tCameraInfo.vCenter.y -  m_tCameraInfo.vPos.y) - m_tCameraInfo.vUp.y * (m_tCameraInfo.vCenter.x - m_tCameraInfo.vPos.x);

		float _len = sqrtf(_x*_x + _y*_y + _z*_z);
		_x/=_len;	_y/=_len;	_z/=_len;
								
		m_tCameraInfo.vCenter.x = m_tCameraInfo.vCenter.x + (m_tCameraInfo.vUp.x * fRel_y * len * m_vWinSize.x/m_vWinSize.y) +
			(_x)* (-fRel_x) * len;
		m_tCameraInfo.vCenter.y = m_tCameraInfo.vCenter.y + (m_tCameraInfo.vUp.y * fRel_y * len * m_vWinSize.x/m_vWinSize.y) +
			(_y)* (-fRel_x) * len;
		m_tCameraInfo.vCenter.z = m_tCameraInfo.vCenter.z + (m_tCameraInfo.vUp.z * fRel_y * len * m_vWinSize.x/m_vWinSize.y) +
			(_z)* (-fRel_x) * len;
	}
	else if(m_bZoom){
		m_tCameraInfo.vPos.x = m_tCameraInfo.vCenter.x + (m_tCameraInfo.vCenter.x - m_tCameraInfo.vPos.x) * (fRel_y-1);
		m_tCameraInfo.vPos.y = m_tCameraInfo.vCenter.y + (m_tCameraInfo.vCenter.y - m_tCameraInfo.vPos.y) * (fRel_y-1);
		m_tCameraInfo.vPos.z = m_tCameraInfo.vCenter.z + (m_tCameraInfo.vCenter.z - m_tCameraInfo.vPos.z) * (fRel_y-1);
	}
}
void CUIEventHandler::OnKeyboardEvent( unsigned char nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_F1: // Change as needed                
			break;
		case 0x52:
			CUIEventHandler::Reset();
			break;
		}
	}

}

void	CUIEventHandler::SetObjectView(D3DXVECTOR3 vMax, D3DXVECTOR3 vMin)
{
	D3DXMATRIX matView;
	D3DXVECTOR3 vCameraDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);//m_vLookVector;
	D3DXVECTOR3 V0 = D3DXVECTOR3( vMax.x, vMax.y, vMin.z ) ;
	D3DXVECTOR3 V1 = D3DXVECTOR3(vMin.x, vMin.y, vMin.z) ;
	D3DXVECTOR3 vCenter = ( V0 + V1 ) * 0.5f;
	float fRadius = (vMax.x-vMin.x ) ;

	float fDistance = fRadius/tanf( DXUTGetWindowWidth() / DXUTGetWindowHeight() * 0.5f );
	D3DXVECTOR3 vPos = vCenter+ vCameraDirection * fDistance;

	m_tCameraInfo.vPos = XMFLOAT3(vPos.x,vPos.y,vPos.z);
	m_tCameraInfo.vCenter = XMFLOAT3(vCenter.x,vCenter.y,vCenter.z);
	m_tCameraInfo.vUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_tRotateInfo.vRotate.x = 0;
	m_tRotateInfo.vRotate.y = 0;
	sTime = eTime=  tTime=0;
}

void	CUIEventHandler::SetLookVector(float _13, float _23, float _33){
	m_vLookVector.x = _13;
	m_vLookVector.y = _23;
	m_vLookVector.z = _33;
};