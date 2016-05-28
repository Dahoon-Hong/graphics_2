#pragma once
#include <DirectXMath.h>
using namespace DirectX;
struct TAABBInfo
{
	TAABBInfo() 
	{
		Clear();
	}
	~TAABBInfo() {}

	void Clear()
	{
		arMin[0] = FLT_MAX; arMin[0] = FLT_MAX; arMin[0] = FLT_MAX;
		arMax[1] = -FLT_MAX; arMax[1] = -FLT_MAX; arMax[1] = -FLT_MAX;
	}
	void Update( float Pos[3] )
	{
		if( arMin[0] > Pos[0] )			arMin[0] = Pos[0];
		if( arMin[1] > Pos[1] )			arMin[1] = Pos[1];
		if( arMin[2] > Pos[2])			arMin[2] = Pos[2];

		if( arMax[0] < Pos[0])			arMax[0] = Pos[0];
		if( arMax[1] < Pos[1])			arMax[1] = Pos[1];
		if( arMax[2] < Pos[2])			arMax[2] = Pos[2];
	}
	void CalCenter()
	{
		arCen[0] = (arMin[0] + arMax[0]) / 2.f;
		arCen[1] = (arMin[1] + arMax[1]) / 2.f;
		arCen[2] = (arMin[2] + arMax[2]) / 2.f;
	}
	float arCen[3];//Center 
	float arMin[3];//min
	float arMax[3];//max
};

struct TRotateInfo
{
	TRotateInfo()
	{
		vRotate		= XMFLOAT2(0.0f, 0.0f);
	}
	XMFLOAT2	vRotate;
};
struct TCameraInfo
{
	TCameraInfo()
	{
		vPos		= XMFLOAT3(0,0,0);
		vCenter		= XMFLOAT3(0,0,0);
		vUp			= XMFLOAT3(0,1,0);

		float fFOV = 0.0;
		float fAspect = 0.0;
		float fNearPlane = 0.0;
		float fFarPlane = 0.0;
	}

	void SetProjectionInfo( float _fFOV, float _fAspect, float _fNearPlane, float _fFarPlane )
	{
		fFOV = _fFOV;
		fAspect = _fAspect;
		fNearPlane = _fNearPlane;
		fFarPlane = _fFarPlane;
	}
	XMFLOAT3	vPos;
	XMFLOAT3	vCenter;
	XMFLOAT3	vUp;

	float fFOV;
	float fAspect;
	float fNearPlane;
	float fFarPlane;
};