#define _XM_NO_INTRINSICS_
#include "DXUT/Core/DXUT.h"
#include "DXUT/Optional/DXUTcamera.h"
#include "DXUT/Optional/SDKmisc.h"
#include "SceneModel.h"
#include "MChallenge.h"

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									  DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return true;
}

ID3D11SamplerState*                 g_pSamplerLinear = NULL;
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									 void* pUserContext )
{
	HRESULT hr = S_OK;
	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext );

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	DXUTCompileFromFile( L"Phong.fx", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob ) ;

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
	if( FAILED( hr ) )
	{    
		SAFE_RELEASE( pVSBlob );
		return hr;
	}

	 // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;
	pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout );
	SAFE_RELEASE( pVSBlob );
	if( FAILED( hr ) )
		return hr;

	// Set the input layout
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob  = nullptr;
	V_RETURN( DXUTCompileFromFile( L"Phong.fx", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob  ) );

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
	SAFE_RELEASE( pPSBlob );
	if( FAILED( hr ) )
		return hr;

	for( int i = 0 ; i < MODEL_COUNT; i++ )
	{
		g_arSceneModel[i].SetDevice( pd3dDevice, g_strFilePathArr[i] );
		bool bImported = g_arSceneModel[i].ImportModel();
		if( bImported )
			g_arSceneModel[i].MakeVBOInfo();
	}

	// Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(TShaderParamEveryFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	V_RETURN( pd3dDevice->CreateBuffer( &bd, nullptr, &g_pBChangesEveryFrame ) );

	bd.ByteWidth = sizeof(TShaderParamNeverChanges);
	V_RETURN( pd3dDevice->CreateBuffer( &bd, nullptr, &g_pBNeverChanges ) );

	bd.ByteWidth = sizeof(TShaderParamMaterialProperty);
	V_RETURN( pd3dDevice->CreateBuffer( &bd, nullptr, &g_pBMaterialProperty ) );

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( pd3dImmediateContext->Map( g_pBNeverChanges, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	auto pCB = reinterpret_cast<TShaderParamNeverChanges*>( MappedResource.pData );
	pCB->vLightDir = g_tSceneInfo.vLightPos;

	pd3dImmediateContext->Unmap( g_pBNeverChanges , 0 );
	
	// Setup the camera's view parameters
	TCameraInfo* tCameraInfo = g_UIEventHandler.GetCameraInfo();
	tCameraInfo->vPos.x = 0.0f; tCameraInfo->vPos.y = 0.0f; tCameraInfo->vPos.z = -500.0f; // eye Pos
	tCameraInfo->vCenter.x = 0.0f; tCameraInfo->vCenter.y = 0.0f; tCameraInfo->vCenter.z = 0.0;
	tCameraInfo->vUp.x = 0.0f; tCameraInfo->vUp.y = 1.0f; tCameraInfo->vUp.z = 0.0;

	g_pBackPlane.Create(pd3dDevice, pd3dImmediateContext);

	return S_OK;
}

HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	// Setup the projection parameters
	float fAspect = static_cast<float>( pBackBufferSurfaceDesc->Width ) / static_cast<float>( pBackBufferSurfaceDesc->Height );

	g_UIEventHandler.GetCameraInfo()->SetProjectionInfo( XM_PI * 0.25f, fAspect, 1.0f, 10000.0f );
	g_UIEventHandler.Initialize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc );
	g_Dialog.SetLocation( pBackBufferSurfaceDesc->Width - 200, pBackBufferSurfaceDesc->Height - 720 );
	g_Dialog.SetSize( 245, 520 );
	return S_OK;
}
void InitUI();
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	WCHAR szTemp[256];

	g_UIEventHandler.disable();

	switch( nControlID )
	{
		// Combo Box 클릭
	case IDC_COMBO_OBJLIST:
		{
			g_nCurModelIdx = ((CDXUTComboBox*)pControl)->GetSelectedIndex();
			TCameraInfo* tCameraInfo = g_UIEventHandler.GetCameraInfo();
			tCameraInfo->max[0] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[0];
			tCameraInfo->max[1] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[1];
			tCameraInfo->max[2] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[2];

			tCameraInfo->min[0] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[0];
			tCameraInfo->min[1] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[1];
			tCameraInfo->min[2] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[2];

			g_UIEventHandler.SetObjectView(tCameraInfo->max, tCameraInfo->min);
		}
		break;
	case SLIDER_R:
	{
		rVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"R-Light %d", (int)rVal );
        g_Dialog.GetStatic( STATIC_R)->SetText( szTemp );
	}
	break;
	case SLIDER_G:
	{
		gVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"G %d", (int)gVal );
        g_Dialog.GetStatic( STATIC_G)->SetText( szTemp );
	}
	break;
	case SLIDER_B:
	{
		bVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"B %d", (int)bVal );
        g_Dialog.GetStatic( STATIC_B)->SetText( szTemp );
	}
	break;
	case SLIDER_X:
	{
		xVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"X-Light %d", (int)xVal );
        g_Dialog.GetStatic( STATIC_X)->SetText( szTemp );
	}
	break;
	case SLIDER_Y:
	{
		yVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"Y %d", (int)yVal );
        g_Dialog.GetStatic( STATIC_Y)->SetText( szTemp );
	}
	break;
	case SLIDER_Z:
	{
		zVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"Z %d", (int)zVal );
        g_Dialog.GetStatic( STATIC_Z)->SetText( szTemp );
	}
	break;

	case SLIDER_D:
	{
		dVal = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"INTENSE %d", (int)dVal );
        g_Dialog.GetStatic( STATIC_D)->SetText( szTemp );
	}
	break;
	case SLIDER_RM:
	{
		rVal_m = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"R-Model %d", (int)rVal_m );
        g_Dialog.GetStatic( STATIC_RM)->SetText( szTemp );
	}
	break;
	case SLIDER_GM:
	{
		gVal_m = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"G %d", (int)gVal_m );
        g_Dialog.GetStatic( STATIC_GM)->SetText( szTemp );
	}
	break;
	case SLIDER_BM:
	{
		bVal_m = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"B %d", (int)bVal_m );
        g_Dialog.GetStatic( STATIC_BM)->SetText( szTemp );
	}
	break;
	case BUTTON_TOGGLE:
	{
		g_UIEventHandler.bSpin = !g_UIEventHandler.bSpin;
	}
	break;
	case SLIDER_XS:
	{
		xVal_s = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"X-SpotLight %d", (int)xVal_s );
        g_Dialog.GetStatic( STATIC_XS)->SetText( szTemp );
	}
	break;
	case SLIDER_YS:
	{
		yVal_s = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"Y %d", (int)yVal_s );
        g_Dialog.GetStatic( STATIC_YS)->SetText( szTemp );
	}
	break;
	case SLIDER_ZS:
	{
		zVal_s = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"Z %d", (int)zVal_s );
        g_Dialog.GetStatic( STATIC_ZS)->SetText( szTemp );
	}
	break;

	case SLIDER_DS:
	{
		dVal_s = ((CDXUTSlider*)pControl)->GetValue();
		swprintf_s( szTemp, L"INTENSE-SpotLight %d", (int)dVal_s );
        g_Dialog.GetStatic( STATIC_DS)->SetText( szTemp );
	}
	break;
	case BUTTON_SETTING:
	{
		xVal = 255;
		yVal = 231;
		zVal = 255;
		dVal = 55;

		xVal_s = 255;
		yVal_s = 255;
		zVal_s = 109;
		dVal_s = 137;
		//InitUI();
	}
	break;
	}
}

void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
}

void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								 double fTime, float fElapsedTime, void* pUserContext )
{
	// Clear the back buffer
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::Gray );

	// Clear the depth stencil
	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	RenderModel( pd3dImmediateContext );
	

	g_Dialog.OnRender( fElapsedTime );
}

void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	SAFE_RELEASE( g_pVertexLayout );
	SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pBNeverChanges );
	SAFE_RELEASE( g_pBChangesEveryFrame );
	SAFE_RELEASE( g_pBMaterialProperty );
	SAFE_RELEASE(g_pSamplerLinear);

	for( int i = 0; i < MODEL_COUNT; i++ )
		g_arSceneModel[i].ReleaseBuffer();
}

LRESULT OnMouse( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return FALSE;
}
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						 bool* pbNoFurtherProcessing, void* pUserContext )
{
	g_UIEventHandler.OnMouseEvent( hWnd, uMsg, wParam, lParam );
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	*pbNoFurtherProcessing = g_Dialog.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	return 0;
}
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	g_UIEventHandler.OnKeyboardEvent( nChar, bKeyDown, bAltDown, pUserContext );
}

bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}

// UI 초기화
void InitUI()
{
	int x,y;
	int b = 20;
	int xShift = 1000;
	WCHAR szTemp[256];
	g_Dialog.Init( &g_DialogResourceManager );

	g_Dialog.SetCallback( OnGUIEvent );
	CDXUTComboBox* pCombo;
	x = 10; y = 25;
	g_Dialog.AddComboBox( IDC_COMBO_OBJLIST, x, y,150, 24, 0, true, &pCombo );
	if( pCombo )
	{
		pCombo->SetDropHeight( 64 );
		pCombo->AddItem( L"Bag", NULL );
		pCombo->AddItem( L"Bus", NULL );
		pCombo->AddItem( L"EyeBall", NULL );
		pCombo->AddItem( L"Scorpion", NULL );
		pCombo->SetSelectedByIndex( 0 );
	}
	y+=100;
	swprintf_s( szTemp, L"R-Light %d", (int)rVal);
	g_Dialog.AddStatic( STATIC_R, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_R, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)rVal, false );

	swprintf_s( szTemp, L"R-Model %d", (int)rVal_m);
	g_Dialog.AddStatic( STATIC_RM, szTemp, x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_RM, x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)rVal_m, false );

	swprintf_s( szTemp, L"G %d", (int)gVal);
	g_Dialog.AddStatic( STATIC_G, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_G, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)gVal, false );

	swprintf_s( szTemp, L"G %d", (int)gVal_m);
	g_Dialog.AddStatic( STATIC_GM, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_GM,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)gVal_m, false );

	swprintf_s( szTemp, L"B %d",(int)bVal);
	g_Dialog.AddStatic( STATIC_B, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_B, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)bVal, false );

	swprintf_s( szTemp, L"B %d",(int)bVal_m);
	g_Dialog.AddStatic( STATIC_BM, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_BM,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)bVal_m, false );

	y+=b;

	swprintf_s( szTemp, L"X-Light %d", (int)xVal);
	g_Dialog.AddStatic( STATIC_X, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_X, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)xVal, false );

	swprintf_s( szTemp, L"X-SpotLight %d", (int)xVal_s);
	g_Dialog.AddStatic( STATIC_XS, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_XS,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)xVal_s, false );

	swprintf_s( szTemp, L"Y %d", (int)yVal );
	g_Dialog.AddStatic( STATIC_Y, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_Y, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)yVal, false );

	swprintf_s( szTemp, L"Y %d", (int)yVal_s );
	g_Dialog.AddStatic( STATIC_YS, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_YS,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)yVal_s, false );

	swprintf_s( szTemp, L"Z %d",(int)zVal);
	g_Dialog.AddStatic( STATIC_Z, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_Z, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)zVal, false );

	swprintf_s( szTemp, L"Z %d",(int)zVal_s);
	g_Dialog.AddStatic( STATIC_ZS, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_ZS,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)zVal_s, false );

	swprintf_s( szTemp, L"INTENSE %d",(int)dVal);
	g_Dialog.AddStatic( STATIC_D, szTemp, x, y+=b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_D, x, y+=b, 140, 24, (int)0, 
                          (int)255, (int)dVal, false );

	swprintf_s( szTemp, L"INTENSE %d",(int)dVal_s);
	g_Dialog.AddStatic( STATIC_DS, szTemp,  x-xShift, y-b, 108, 24 );
	g_Dialog.AddSlider( SLIDER_DS,  x-xShift, y, 140, 24, (int)0, 
                          (int)255, (int)dVal_s, false );

	y+=2*b;
	g_Dialog.AddButton( BUTTON_TOGGLE, L"Toggle Spin", x, y, 125, 22 );
	g_Dialog.AddButton( BUTTON_SETTING, L"Set View", x-xShift, y, 125, 22 );
	

}
void RenderBackGround(ID3D11DeviceContext* deviceContext);
void RenderModel(ID3D11DeviceContext* deviceContext)
{
	
	// Get the projection & view matrix from the camera class
	XMVECTORF32 AxisX = { 1,0,0 }; XMVECTORF32 AxisY = { 0,1,0 };
	XMMATRIX mRotateX = XMMatrixRotationAxis( AxisX, g_UIEventHandler.GetRotateX() );
	XMMATRIX mRotateY = XMMatrixRotationAxis( AxisY, g_UIEventHandler.GetRotateY() );
	if(g_UIEventHandler.bSpin){
		g_UIEventHandler.eTime = clock();
		g_UIEventHandler.tTime = g_UIEventHandler.eTime - g_UIEventHandler.sTime;
	}
	if(!g_UIEventHandler.bSpin){
		g_UIEventHandler.eTime = clock();
		g_UIEventHandler.sTime = clock() - g_UIEventHandler.tTime;
	}
	XMMATRIX mWorld = mRotateY * mRotateX;
	
	mWorld *= XMMatrixRotationY(float(g_UIEventHandler.eTime-g_UIEventHandler.sTime)/(CLOCKS_PER_SEC)) * XMMatrixTranslation(0,10*sin(float(g_UIEventHandler.eTime-g_UIEventHandler.sTime)/(CLOCKS_PER_SEC)*5),0);
	//mWorld *= XMMatrixRotationY(float(eTime-sTime)/(CLOCKS_PER_SEC)) * XMMatrixTranslation(0,10*sin(float(eTime-sTime)/(CLOCKS_PER_SEC)*5),0);
	TCameraInfo* tCameraInfo = g_UIEventHandler.GetCameraInfo();

	XMVECTOR vEye = XMLoadFloat3( &tCameraInfo->vPos );
	XMVECTOR vCenter =  XMLoadFloat3( &tCameraInfo->vCenter );
	XMVECTOR vUp =  XMLoadFloat3( &tCameraInfo->vUp );

	XMMATRIX mView = XMMatrixLookAtLH( vEye, vCenter, vUp ) ;;
	XMMATRIX mProj = XMMatrixPerspectiveFovLH( tCameraInfo->fFOV, tCameraInfo->fAspect, tCameraInfo->fNearPlane, tCameraInfo->fFarPlane );
	XMMATRIX mWorldViewProjection = mWorld * mView * mProj;
	XMMATRIX mWorldView = mWorld * mView;
	
	mView.m[3][0] = 0;
	mView.m[3][1] = 0;
	mView.m[3][2] = 0;
	XMMATRIX mWorld2 = XMMatrixScaling(800.0f, 500.0f, 100.0f);
	XMMATRIX mProj2 = XMMatrixPerspectiveFovLH( tCameraInfo->fFOV, tCameraInfo->fAspect, tCameraInfo->fNearPlane, tCameraInfo->fFarPlane );
	XMMATRIX mWorldViewProjection2 =  mWorld2*mProj2;

	// Update constant buffer that changes once per frame
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( deviceContext->Map( g_pBChangesEveryFrame , 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	auto pCB = reinterpret_cast<TShaderParamEveryFrame*>( MappedResource.pData );
	
	XMStoreFloat4x4( &pCB->mWorldViewProj2, XMMatrixTranspose( mWorldViewProjection2 ) );
	XMStoreFloat4x4( &pCB->mWorldViewProj, XMMatrixTranspose( mWorldViewProjection ) );
	XMStoreFloat4x4( &pCB->mWorld, XMMatrixTranspose( mWorld ) );
	XMStoreFloat4x4( &pCB->mWorldView, XMMatrixTranspose( mWorldView ) );

	//Camera Pos
	pCB->vEye.x = tCameraInfo->vPos.x;	pCB->vEye.y = tCameraInfo->vPos.y;
	pCB->vEye.z = tCameraInfo->vPos.z;	pCB->vEye.w=1.0f;
	float temp = float(dVal)/255.0f*2.0f;
	pCB->vLightPoint1=XMFLOAT4((float)xVal/255.0f*2.0f-1.0f,(float)yVal/255.0f*2.0f-1.0f,(float)zVal/255.0f*2.0f-1.0f,1.0f);
	pCB->vLightPoint2=XMFLOAT4(1000.0f,1000.0f,1000.0f,1.0f);
	temp = float(dVal_s)/255.0f*6.0f;
	pCB->vLightPoint3=XMFLOAT4((float)xVal_s/255.0f*temp-temp/2.0f,(float)yVal_s/255.0f*temp-temp/2.0f,(float)zVal_s/255.0f*temp-temp/2.0f,1.0f);
	pCB->vrgbV = XMFLOAT4((float)rVal/255.0f,(float)gVal/255.0f,(float)bVal/255.0f,1.0f);
	pCB->vrgbV_m = XMFLOAT4((float)rVal_m/255.0f,(float)gVal_m/255.0f,(float)bVal_m/255.0f,1.0f);
	pCB->vintense = dVal/255.0f;
	deviceContext->Unmap( g_pBChangesEveryFrame , 0 );

	deviceContext->IASetInputLayout( g_pVertexLayout );
	g_UIEventHandler.SetView( g_arSceneModel[g_nCurModelIdx].GetAABB().arMax,g_arSceneModel[g_nCurModelIdx].GetAABB().arMin);
	g_UIEventHandler.SetLookVector(mView._13,mView._23,mView._33);


	tCameraInfo->max[0] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[0];
	tCameraInfo->max[1] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[1];
	tCameraInfo->max[2] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMax[2];

	tCameraInfo->min[0] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[0];
	tCameraInfo->min[1] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[1];
	tCameraInfo->min[2] = g_arSceneModel[g_nCurModelIdx].GetAABB().arMin[2];


	RenderBackGround(deviceContext);
	
	RenderModelInstList(deviceContext, g_arSceneModel[g_nCurModelIdx].GetMeshOpaqList() );
	RenderModelInstList(deviceContext, g_arSceneModel[g_nCurModelIdx].GetMeshTransList() );

	return;
}

void RenderBackGround(ID3D11DeviceContext* deviceContext)
{
	g_pBackPlane.Render();
	//g_pBackPlane.m_VB.Apply(deviceContext);
	//g_pBackPlane.m_IB.Apply(deviceContext);
	//
	//ID3D11ShaderResourceView* pSRV = NULL;
	////
	////pSRV = g_pBackPlane.m_pTexture->GetShaderResourceView();
	////deviceContext->PSSetShaderResources( 2, 1,  &pSRV);

	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//deviceContext->DrawIndexed( ( UINT )6, 0, ( UINT )0);
}

void RenderModelInstList(ID3D11DeviceContext* deviceContext, std::vector < TVBOInfo* > arInstList  )
{
	unsigned int stride = sizeof(TDrawVertexType); 
	unsigned int offset = 0;
	std::map< TVBOInfo*, TMaterialInfo* > pTMaterialMap = g_arSceneModel[g_nCurModelIdx].GetMeshMaterialMap();

	for( int i = 0 ; i < arInstList.size(); i++ )
	{
		deviceContext->RSSetState(g_arSceneModel[g_nCurModelIdx].GetRasterStateBuffer());
		float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		deviceContext->OMSetBlendState(g_arSceneModel[g_nCurModelIdx].GetBlendStateBuffer(), BlendFactor, 0xFFFFFFFF );
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		TVBOInfo * pVBO = arInstList.at(i);

		ID3D11Buffer * vertexbuffer = arInstList.at(i)->VertexBuffer;
		ID3D11Buffer * indexbuffer = arInstList.at(i)->IndexBuffer;

		// Material 연결
		TMaterialInfo * pTMaterial = pTMaterialMap.at( pVBO );

		int iStoreTex = 0;
		ID3D11ShaderResourceView* pSRV = NULL;
		if(pTMaterial->pAlphaTex != NULL)
		{
			iStoreTex +=1;

			pSRV = pTMaterial->pAlphaTex->GetShaderResourceView();
			deviceContext->PSSetShaderResources( 0, 1,  &pSRV);
		}
		if(pTMaterial->pAmbientTex != NULL)
		{
			iStoreTex +=2;
			pSRV = pTMaterial->pAmbientTex->GetShaderResourceView();
			deviceContext->PSSetShaderResources( 1, 1,  &pSRV);
		}
		if(pTMaterial->pDiffuseTex != NULL)
		{
			iStoreTex +=4;
			pSRV = pTMaterial->pDiffuseTex->GetShaderResourceView();
			deviceContext->PSSetShaderResources( 2, 1,  &pSRV);
		}
		if(pTMaterial->pSpecTex != NULL)
		{
			iStoreTex +=8;
			pSRV = pTMaterial->pSpecTex->GetShaderResourceView();
			deviceContext->PSSetShaderResources( 3, 1,  &pSRV);
		}
		if(pTMaterial->pNormalTex != NULL)
		{
			iStoreTex +=16;
			pSRV =  pTMaterial->pNormalTex->GetShaderResourceView();
			deviceContext->PSSetShaderResources( 4, 1, &pSRV);
		}
		

		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		V( deviceContext->Map( g_pBMaterialProperty , 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
		auto pCB = reinterpret_cast<TShaderParamMaterialProperty*>( MappedResource.pData );
		pCB->vEmissive = pTMaterial->Emissive;
		pCB->vAmbient = pTMaterial->Ambient;
		pCB->vDiffuse = pTMaterial->Diffuse;
		pCB->vSpecular = pTMaterial->Specular;
		pCB->fShininess = pTMaterial->Shininess;
		pCB->iStoreTex	= iStoreTex;
		pCB->Opacity = pTMaterial->Opacity;
		deviceContext->Unmap( g_pBMaterialProperty , 0 );


		// Buffer 연결
		deviceContext->IASetVertexBuffers(0, 1, &vertexbuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(indexbuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->VSSetShader( g_pVertexShader, nullptr, 0 );
		deviceContext->VSSetConstantBuffers( 0, 1, &g_pBNeverChanges );
		deviceContext->VSSetConstantBuffers( 1, 1, &g_pBChangesEveryFrame );
		deviceContext->VSSetConstantBuffers( 2, 1, &g_pBMaterialProperty );

		deviceContext->PSSetShader( g_pPixelShader, nullptr, 0 );
		deviceContext->PSSetConstantBuffers( 0, 1, &g_pBNeverChanges );
		deviceContext->PSSetConstantBuffers( 1, 1, &g_pBChangesEveryFrame );
		deviceContext->PSSetConstantBuffers( 2, 1, &g_pBMaterialProperty );


		// 	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		deviceContext->DrawIndexed( ( UINT )pVBO->nIdxCount, 0, ( UINT )0);
	}
}

int main( )
{
	g_nCurModelIdx = 0;
	g_UIEventHandler.sTime=clock();
	g_UIEventHandler.tTime=clock();
	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here
	InitUI();
	DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"Midas Challenge 3D Model Viewer" );

	// Only require 10-level hardware or later
	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1200, 700 );
	DXUTMainLoop(); // Enter into the DXUT render loop

	// Perform any application-level cleanup here
	return DXUTGetExitCode();
}