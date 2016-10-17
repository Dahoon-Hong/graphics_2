#pragma once
#include <DirectXMath.h>
#include <windef.h>
#include "DataTypeDef.h"
#include <D3DX9Math.h>
#include <time.h>
using namespace DirectX;

class CUIEventHandler
{
public:
	CUIEventHandler(void);
	virtual ~CUIEventHandler(void);

public:
	void Initialize( int nWinXSize, int nWinYSize );

	void OnMouseEvent( HWND , UINT uMsg, WPARAM wParam, LPARAM lParam );
	void OnMouseMotionEvent( int nWinX, int nWinY );
	void OnKeyboardEvent( unsigned char nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
	void Reset();
	float GetRotateX(){ return  m_tRotateInfo.vRotate.x ; }
	float GetRotateY(){ return  m_tRotateInfo.vRotate.y ; }
	void SetView(D3DXVECTOR3 vMax, D3DXVECTOR3 vMin){
		_vMax = vMax; _vMin = vMin;
	};
	void	CUIEventHandler::SetLookVector(float _13, float _23, float _33);
	TCameraInfo*	GetCameraInfo() { return &m_tCameraInfo; };
	void			SetObjectView(D3DXVECTOR3 vMax, D3DXVECTOR3 vMin);
	void			disable(){m_bRotating = false;};
//	void			camReset(time_t &sTime,time_t &eTime,time_t &tTime){sTime =eTime = tTime = clock();return;};
	time_t			sTime, eTime, tTime;
	bool			 bSpin;
protected:
	bool			m_bRotating;
	bool			m_bZoom;
	bool			m_bPanning;

	TRotateInfo		m_tRotateInfo;
	TCameraInfo		m_tCameraInfo;

	TRotateInfo		m_tRotateInit;
	TCameraInfo		m_tCameraInit;
	XMINT2			m_vPosNew, m_vPosOld;
	XMINT2			m_vWinSize;


	XMFLOAT3		m_vLookVector;
	D3DXVECTOR3		_vMax,_vMin;

};

