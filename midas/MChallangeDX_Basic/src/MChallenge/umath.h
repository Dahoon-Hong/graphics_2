#include<DirectXMath.h>
#include "SceneModel.h"
XMFLOAT4 operator*(XMFLOAT4X4 left, XMFLOAT4 right){

	return XMFLOAT4(left._11*right.x + left._12*right.y + left._13*right.z + left._14*right.w,
		left._21*right.x + left._22*right.y + left._23*right.z + left._24*right.w,
		left._31*right.x + left._32*right.y + left._33*right.z + left._34*right.w,
		left._41*right.x + left._42*right.y + left._43*right.z + left._44*right.w
		);
}

XMFLOAT4X4 operator*(XMFLOAT4X4 left, XMFLOAT4X4 right){

	float _11 = left._11*right._11 + left._12*right._21 + left._13*right._31 + left._14*right._41;
	float _12 = left._11*right._12 + left._12*right._22 + left._13*right._32 + left._14*right._42;
	float _13 = left._11*right._13 + left._12*right._23 + left._13*right._33 + left._14*right._43;
	float _14 = left._11*right._14 + left._12*right._24 + left._13*right._34 + left._14*right._44;

	float _21 = left._21*right._11 + left._22*right._21 + left._23*right._31 + left._24*right._41;
	float _22 = left._21*right._12 + left._22*right._22 + left._23*right._32 + left._24*right._42;
	float _23 = left._21*right._13 + left._22*right._23 + left._23*right._33 + left._24*right._43;
	float _24 = left._21*right._14 + left._22*right._24 + left._23*right._34 + left._24*right._44;

	float _31 = left._31*right._11 + left._32*right._21 + left._33*right._31 + left._34*right._41;
	float _32 = left._31*right._12 + left._32*right._22 + left._33*right._32 + left._34*right._42;
	float _33 = left._31*right._13 + left._32*right._23 + left._33*right._33 + left._34*right._43;
	float _34 = left._31*right._14 + left._32*right._24 + left._33*right._34 + left._34*right._44;

	float _41 = left._41*right._11 + left._42*right._21 + left._43*right._31 + left._44*right._41;
	float _42 = left._41*right._12 + left._42*right._22 + left._43*right._32 + left._44*right._42;
	float _43 = left._41*right._13 + left._42*right._23 + left._43*right._33 + left._44*right._43;
	float _44 = left._41*right._14 + left._42*right._24 + left._43*right._34 + left._44*right._44;

	return XMFLOAT4X4(_11,_12,_13,_14,
		_21,_22,_23,_24,
		_31,_32,_33,_34,
		_41,_42,_43,_44);
}
XMFLOAT3 assign(XMFLOAT4 right){
	return XMFLOAT3(right.x, right.y,right.z);
}

XMFLOAT4X4 rotate( XMFLOAT3 axis, float angle )
{
	float c=cos(angle), s=sin(angle), x=axis.x, y=axis.y, z=axis.z;
	float _11 = x*x*(1-c)+c;		float _12 = x*y*(1-c)-z*s;		float _13 = x*z*(1-c)+y*s;	float _14 = 0.0f;
	float _21 = x*y*(1-c)+z*s;	float _22 = y*y*(1-c)+c;			float _23 = y*z*(1-c)-x*s;	float _24 = 0.0f;
	float _31 = x*z*(1-c)-y*s;	float _32 = y*z*(1-c)+x*s;		float _33 = z*z*(1-c)+c;		float _34 = 0.0f;
	float _41 = 0;				float _42 = 0;					float _43 = 0;				float _44 = 1.0f;
	return XMFLOAT4X4(	_11,_21,_31,_41,
						_12,_22,_32,_42,
						_13,_23,_33,_43,
						_14,_24,_34,_44);
}

XMFLOAT3 operator*(XMFLOAT3 left, float right){
	return XMFLOAT3(left.x * right, left.y * right, left.z*right);
}



/*
cam.eye = (mat4::translate(cam.at) * mat4::rotate(cam.up, -p1.x*3.14f) * mat4::translate(-cam.at)) * vec4(cam.eye, 1);
			cam.eye = (mat4::translate(cam.at) * mat4::rotate((cross(cam.up, cam.at - cam.eye)).normalize(), p1.y*3.14f) * mat4::translate(-cam.at)) * vec4(cam.eye, 1);
			cam.up = ((mat4::translate(cam.at) * mat4::rotate((cross(cam.up, cam.at - cam.eye)).normalize(), p1.y*3.14f) * mat4::translate(-cam.at)) * vec4(cam.up, 0)).normalize();
			*/