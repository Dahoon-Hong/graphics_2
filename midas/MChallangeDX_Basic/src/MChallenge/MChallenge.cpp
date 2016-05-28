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

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	
	g_tSceneInfo.vLightPos[0].x+=100.0f;
	pCB->vLightDir[0] = g_tSceneInfo.vLightPos[0];
	g_tSceneInfo.vLightPos[1].x-=100.0f;
	pCB->vLightDir[1]=g_tSceneInfo.vLightPos[1];
	g_tSceneInfo.vLightPos[2].z-=100.0f;
	pCB->vLightDir[2]=g_tSceneInfo.vLightPos[2];
	pCB->vLightColor[0]=g_tSceneInfo.vGlobalAmbient[0];
	pCB->vLightColor[0].x=1.0f;
	pCB->vLightColor[1]=g_tSceneInfo.vGlobalAmbient[1];
	pCB->vLightColor[1].x=1.0f;
	pCB->vLightColor[2]=g_tSceneInfo.vGlobalAmbient[2];
	pCB->vLightColor[2].y=1.0f;

    pd3dImmediateContext->Unmap( g_pBNeverChanges , 0 );

	// Setup the camera's view parameters
	TCameraInfo* tCameraInfo = g_UIEventHandler.GetCameraInfo();
	tCameraInfo->vPos.x = 0.0f; tCameraInfo->vPos.y = 0.0f; tCameraInfo->vPos.z = -500.0f; // eye Pos
	tCameraInfo->vCenter.x = 0.0f; tCameraInfo->vCenter.y = 0.0f; tCameraInfo->vCenter.z = 0.0;
	tCameraInfo->vUp.x = 0.0f; tCameraInfo->vUp.y = 1.0f; tCameraInfo->vUp.z = 0.0;

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

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	switch( nControlID )
	{
	// Combo Box 클릭
	case IDC_COMBO_OBJLIST:
		{
			g_nCurModelIdx = ((CDXUTComboBox*)pControl)->GetSelectedIndex();
		}
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
    pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::White );

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
	g_Dialog.Init( &g_DialogResourceManager );

	g_Dialog.SetCallback( OnGUIEvent );
	CDXUTComboBox* pCombo;
	g_Dialog.AddComboBox( IDC_COMBO_OBJLIST, 0, 10, 125, 24, 0, true, &pCombo );
	if( pCombo )
	{
		pCombo->SetDropHeight( 64 );
		pCombo->AddItem( L"Bag", NULL );
		pCombo->AddItem( L"Bus", NULL );
		pCombo->AddItem( L"EyeBall", NULL );
		pCombo->AddItem( L"Scorpion", NULL );
		pCombo->SetSelectedByIndex( 0 );
	}
}
 void RenderModel(ID3D11DeviceContext* deviceContext)
{
	// Get the projection & view matrix from the camera class
	XMVECTORF32 AxisX = { 1,0,0 }; XMVECTORF32 AxisY = { 0,1,0 };
	XMMATRIX mRotateX = XMMatrixRotationAxis( AxisX, g_UIEventHandler.GetRotateX() );
	XMMATRIX mRotateY = XMMatrixRotationAxis( AxisY, g_UIEventHandler.GetRotateY() );

	XMMATRIX mWorld = mRotateY * mRotateX;
	TCameraInfo* tCameraInfo = g_UIEventHandler.GetCameraInfo();

	XMVECTOR vEye = XMLoadFloat3( &tCameraInfo->vPos );
	XMVECTOR vCenter =  XMLoadFloat3( &tCameraInfo->vCenter );
	XMVECTOR vUp =  XMLoadFloat3( &tCameraInfo->vUp );

	XMMATRIX mView = XMMatrixLookAtLH( vEye, vCenter, vUp );
	XMMATRIX mProj = XMMatrixPerspectiveFovLH( tCameraInfo->fFOV, tCameraInfo->fAspect, tCameraInfo->fNearPlane, tCameraInfo->fFarPlane );
	XMMATRIX mWorldViewProjection = mWorld * mView * mProj;
	XMMATRIX mWorldView = mWorld * mView;

	// Update constant buffer that changes once per frame
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( deviceContext->Map( g_pBChangesEveryFrame , 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	auto pCB = reinterpret_cast<TShaderParamEveryFrame*>( MappedResource.pData );
	XMStoreFloat4x4( &pCB->mWorldViewProj, XMMatrixTranspose( mWorldViewProjection ) );
	XMStoreFloat4x4( &pCB->mWorld, XMMatrixTranspose( mWorld ) );
	XMStoreFloat4x4( &pCB->mWorldView, XMMatrixTranspose( mWorldView ) );
	
	//Camera Pos
	pCB->vEye.x = tCameraInfo->vPos.x;	pCB->vEye.y = tCameraInfo->vPos.y;
	pCB->vEye.z = tCameraInfo->vPos.z;	pCB->vEye.w = 1.0f;

	deviceContext->Unmap( g_pBChangesEveryFrame , 0 );

	deviceContext->IASetInputLayout( g_pVertexLayout );

	RenderModelInstList(deviceContext, g_arSceneModel[g_nCurModelIdx].GetMeshOpaqList() );
	RenderModelInstList(deviceContext, g_arSceneModel[g_nCurModelIdx].GetMeshTransList() );

	return;
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

		 HRESULT hr;
		 D3D11_MAPPED_SUBRESOURCE MappedResource;
		 V( deviceContext->Map( g_pBMaterialProperty , 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
		 auto pCB = reinterpret_cast<TShaderParamMaterialProperty*>( MappedResource.pData );
		 // 이곳에서 메터리얼 정보를 넘겨줌
		 pCB->vAmbient = pTMaterial->Emissive;
		 pCB->vAmbient = pTMaterial->Ambient;
		 pCB->vDiffuse = pTMaterial->Diffuse;
		 pCB->vSpecular = pTMaterial->Specular;
		 pCB->fShininess = pTMaterial->Shininess;
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
	 DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1024, 768 );
	 DXUTMainLoop(); // Enter into the DXUT render loop

	 // Perform any application-level cleanup here
	 return DXUTGetExitCode();
 }