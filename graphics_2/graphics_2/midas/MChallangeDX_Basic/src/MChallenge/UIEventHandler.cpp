#include <Windows.h>
#include "UIEventHandler.h"
#include <d3dx9math.h>

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
		m_tRotateInfo.vRotate.y -= D3DXToRadian(fRel_x * 180 );
		m_tRotateInfo.vRotate.x -= D3DXToRadian(fRel_y * 180);
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
		}
	}

}

