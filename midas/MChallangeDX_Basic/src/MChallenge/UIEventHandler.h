#pragma once
#include <DirectXMath.h>
#include <windef.h>
#include "DataTypeDef.h"

using namespace DirectX;

class CUIEventHandler
{
public:
	CUIEventHandler(void);
	virtual ~CUIEventHandler(void);

public:
	void Initialize( int nWinXSize, int nWinYSize );

	void OnMouseEvent( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void OnMouseMotionEvent( int nWinX, int nWinY );
	void OnKeyboardEvent( unsigned char nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
	float GetRotateX(){ return  m_tRotateInfo.vRotate.x ; }
	float GetRotateY(){ return  m_tRotateInfo.vRotate.y ; }

	TCameraInfo* GetCameraInfo() { return &m_tCameraInfo; }

protected:
	bool			m_bRotating;
	TRotateInfo		m_tRotateInfo;
	TCameraInfo		m_tCameraInfo;
	XMINT2			m_vPosNew, m_vPosOld;
	XMINT2			m_vWinSize;
};

