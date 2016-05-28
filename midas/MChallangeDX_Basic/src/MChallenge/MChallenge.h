#include "DXUT/Optional/DXUTcamera.h"
#include <DXUT/Optional/DXUTgui.h>
#include <DirectXMath.h>
#include "SceneModel.h"
#include "UIEventHandler.h"

#define IDC_COMBO_OBJLIST                1

using namespace DirectX;

struct TShaderParamNeverChanges
{
	XMFLOAT4 vLightDir;
};

struct TShaderParamEveryFrame
{
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 mWorldView;
	XMFLOAT4X4 mWorld;
	XMFLOAT4   vEye;
};

struct TShaderParamMaterialProperty
{
	XMFLOAT4  vEmissive;      
	XMFLOAT4  vAmbient;        
	XMFLOAT4  vDiffuse;        
	XMFLOAT4  vSpecular;       
	float	  fShininess;
	int		  iStoreTex;
	float	  fPadding[2];
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


CDXUTDialog                 g_Dialog;
CDXUTDialogResourceManager  g_DialogResourceManager;
int							g_nCurModelIdx;
CUIEventHandler				g_UIEventHandler;
void RenderModel(ID3D11DeviceContext* deviceContext);
void RenderModelInstList(ID3D11DeviceContext* deviceContext, std::vector < TVBOInfo* > arInstList );
