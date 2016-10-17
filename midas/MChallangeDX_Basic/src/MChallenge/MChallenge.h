#define _XM_NO_INTRINSICS_
#include "DXUT/Optional/DXUTcamera.h"
#include <DXUT/Optional/DXUTgui.h>
#include <DirectXMath.h>
#include "SceneModel.h"
#include "UIEventHandler.h"
#include "OBackGroundPlane.h"
#include <time.h>

#define IDC_COMBO_OBJLIST                1
#define SLIDER_R 2
#define SLIDER_G 3
#define SLIDER_B 4

#define STATIC_R 5
#define STATIC_G 6
#define STATIC_B 7

#define SLIDER_X 8
#define SLIDER_Y 9
#define SLIDER_Z 10
#define SLIDER_D 11

#define STATIC_X 12
#define STATIC_Y 13
#define STATIC_Z 14
#define STATIC_D 15

#define SLIDER_RM 16
#define SLIDER_GM 17
#define SLIDER_BM 18

#define STATIC_RM 19
#define STATIC_GM 20
#define STATIC_BM 21

#define BUTTON_TOGGLE 22

#define SLIDER_XS 23
#define SLIDER_YS 24
#define SLIDER_ZS 25
#define SLIDER_DS 26

#define STATIC_XS 27
#define STATIC_YS 28
#define STATIC_ZS 29
#define STATIC_DS 30

#define BUTTON_SETTING 31



using namespace DirectX;

struct TShaderParamNeverChanges
{
	XMFLOAT4 vLightDir;
};

struct TShaderParamEveryFrame
{

	XMFLOAT4X4	mWorldViewProj2;
	XMFLOAT4X4	mWorldViewProj;
	XMFLOAT4X4	mWorldView;
	XMFLOAT4X4	mWorld;
	XMFLOAT4	vEye;
	XMFLOAT4	vLightPoint1;
	XMFLOAT4	vLightPoint2;
	XMFLOAT4	vLightPoint3;
	XMFLOAT4	vrgbV;
	XMFLOAT4	vrgbV_m;
	float vintense;
	float vpadding[3];
};

struct TShaderParamMaterialProperty
{
	XMFLOAT4  vEmissive;      
	XMFLOAT4  vAmbient;        
	XMFLOAT4  vDiffuse;        
	XMFLOAT4  vSpecular;       
	float	  fShininess;
	int		  iStoreTex;
	float		Opacity;
	float	  fPadding;
};



struct TSceneInfo
{
	TSceneInfo()
	{
		vLightPos = XMFLOAT4(1.0f, 1.0f, -1.0f, 0.0f);
		vGlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
	}
	XMFLOAT4					vGlobalAmbient;
	XMFLOAT4					vLightPos;
};



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
#define MODEL_COUNT 4

std::string					g_strFilePathArr[MODEL_COUNT] = { 
								"Models\\alicebag\\Bag.obj",   
								"Models\\Bus\\Bus.obj", 
								"Models\\EYE\\eyeball.obj",
								"Models\\Scorpion\\scorpion2.obj",
								
							};

CSceneModel					g_arSceneModel[MODEL_COUNT];

ID3D11VertexShader*         g_pVertexShader = nullptr;
ID3D11PixelShader*          g_pPixelShader = nullptr;
ID3D11InputLayout*          g_pVertexLayout = nullptr;
ID3D11Buffer*               g_pBNeverChanges = nullptr;
ID3D11Buffer*               g_pBChangesEveryFrame = nullptr;
ID3D11Buffer*               g_pBMaterialProperty = nullptr;

XMMATRIX                    g_matWorld;
XMMATRIX                    g_matView;
XMMATRIX                    g_matProjection;

TSceneInfo					g_tSceneInfo;

OBackGroundPlane			g_pBackPlane;

CDXUTDialog                 g_Dialog;
CDXUTDialogResourceManager  g_DialogResourceManager;
int							g_nCurModelIdx;
CUIEventHandler				g_UIEventHandler;
void RenderModel(ID3D11DeviceContext* deviceContext);
void RenderModelInstList(ID3D11DeviceContext* deviceContext, std::vector < TVBOInfo* > arInstList );

float rVal = 255, gVal=255, bVal=255;
float rVal_m = 255, gVal_m = 255, bVal_m = 255;
float xVal = 255, yVal = 255, zVal = 255, dVal = 255/2;
float xVal_s = 138.0f, yVal_s = 255.0f, zVal_s = 160.0f, dVal_s =253;





