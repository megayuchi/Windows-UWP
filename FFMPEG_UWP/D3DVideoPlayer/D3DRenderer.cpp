#include "pch.h"
#include <d3d11.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "d3d_typede.h"
#include "D3DHelper.h"
#include "D3DRenderer.h"

#include "../Common_UTIL/debug_new.h"

#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "DXGI.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "D3DCompiler.lib" )

using namespace DirectX;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace D2D1;
using namespace Windows::UI::Xaml::Controls;


CD3DRenderer::CD3DRenderer()
{

	m_compositionScaleX = 1.0f;
	m_compositionScaleY = 1.0f;
	m_dpi = 96.0f;

	m_dwWidth = 0;
	m_dwHeight = 0;
	m_dwCreateFlags = 0;


	m_FillMode = D3D11_FILL_SOLID;
	m_FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	m_driverType = D3D_DRIVER_TYPE_UNKNOWN;
	m_pSwapChainPanelNative = NULL;

	// D3D
	m_pD3DDevice = NULL;
	m_pImmediateContext = NULL;
	
	// D2D
	m_pD2DDevice = NULL;
	m_pD2DDeviceContext = NULL;
	m_pD2DTargetBitmap = NULL;
	
	// DWRITE
	m_pDWriteFactory = NULL;
	m_pTextBrush = NULL;
	m_pTextFormat = NULL;
	
	// Back Bufer
	m_pSwapChain = NULL;
	m_pBackBuffer = NULL;
	m_pDiffuseRTV = NULL;
	m_pDSV = NULL;
	m_pDepthStencil = NULL;

	m_dwTextureWidth = 0;
	m_dwTextureHeight = 0;
	
	m_pYUVTexture = NULL;
	m_pYUVTextureSRV = NULL;

	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_pVertexLayout = NULL;
	m_pVS = NULL;
	memset(m_pPS,0,sizeof(m_pPS));

	memset(m_ppDepthStencilState,0,sizeof(m_ppDepthStencilState));
	memset(m_ppBlendState,0,sizeof(m_ppBlendState));
	memset(m_ppRasterizeState,0,sizeof(m_ppRasterizeState));
	memset(m_ppSamplerState,0,sizeof(m_ppSamplerState));

	m_CurShaderFilter = PIXEL_SHADER_FILTER_DEFAULT;
}

BOOL CD3DRenderer::Initialize(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel,DWORD dwFlags)
{
	BOOL	bResult = FALSE;

	HRESULT hr = S_OK;

	m_dwCreateFlags = dwFlags;

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	float dpi = currentDisplayInformation->LogicalDpi;
	
	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;

	SetSwapChainPanel(swapChainPanel);

	// Calculate the necessary swap chain and render target size in pixels.
	m_outputSize.Width = m_logicalSize.Width * m_compositionScaleX;
	m_outputSize.Height = m_logicalSize.Height * m_compositionScaleY;
	
	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);


	m_d3dRenderTargetSize.Width = m_outputSize.Width;
	m_d3dRenderTargetSize.Height = m_outputSize.Height;

	UINT	Width = lround(m_d3dRenderTargetSize.Width);
	UINT	Height = lround(m_d3dRenderTargetSize.Height);

	CreateD3D();
	CreateSwapChain(swapChainPanel,Width,Height);
	CreateBackBuffer(Width,Height);
	CreateD2D(dpi);
	CreateDWrite();

	D3D11_DEPTH_STENCIL_DESC	depthDesc;
	SetDefaultDepthStencilValue(&depthDesc);

	struct DEPTH_TYPE
	{
		BOOL	bDisableWrite;
		BOOL	bDisableTest;
	};

	DEPTH_TYPE	depthTypeList[MAX_DEPTH_TYPE_NUM] = 
	{
		FALSE,FALSE,
		TRUE,FALSE,
		TRUE,TRUE,
		FALSE,TRUE
	};
	for (DWORD i=0; i<MAX_DEPTH_TYPE_NUM; i++)
	{
		if (depthTypeList[i].bDisableWrite)
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		else
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
				

		if (depthTypeList[i].bDisableTest)
			depthDesc.DepthEnable = FALSE;
		else
			depthDesc.DepthEnable = TRUE;
		
			

		DWORD	dwIndex = GetDepthTypeIndex(depthTypeList[i].bDisableWrite,depthTypeList[i].bDisableTest);
			
		hr = m_pD3DDevice->CreateDepthStencilState(&depthDesc,m_ppDepthStencilState+dwIndex);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"Create() - File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}
	}

	m_pImmediateContext->OMSetDepthStencilState(m_ppDepthStencilState[0],0);

	CreateStates();

#ifdef _DEBUG
	SetD3DDebugSetting();
#endif
	
	InitBuffer();
	InitShader();

	bResult = TRUE;
lb_return:

	return bResult;

}
void CD3DRenderer::CreateD3D()
{
	

	HRESULT hr;

	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG) && !defined(ARM_ARCH)
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	//creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//D3D_FEATURE_LEVEL featureLevels[] = 
	//{
	//	D3D_FEATURE_LEVEL_9_3
	//};
	
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_1
	};
	
	IDXGIAdapter* pAdapter = NULL; 
	IDXGIFactory1* pFactory = NULL;

	DXGI_ADAPTER_DESC adapterDesc = { 0 };
	ZeroMemory(&adapterDesc, sizeof(adapterDesc));

	//! Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &pFactory)))
	{
		__debugbreak();
	}
	DWORD	dwDeviceCount = 0;
		
	
	// nvidia + optimus인 경우 종료시 d3device의 refcount가 2 남음.CreateBackBuffer()에서 3이 증가하고 CleanupBackBuffer()에서 1이 감소함.
	// intel or ms basic device의 경우 이 현상이 없음.
	const	UINT VENDOR_ID_NVIDIA = 4318;
	const	UINT VENDOR_ID_INTEL = 32902;
	// VendorId , intel 32902
	// VendorId , nvidia 4318	
	while (pFactory->EnumAdapters(dwDeviceCount, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		// Get adapter description
		pAdapter->GetDesc(&adapterDesc);
	
		if (VENDOR_ID_INTEL == adapterDesc.VendorId)
		{
			break;
		}
		pAdapter->Release();
		pAdapter = NULL;

		dwDeviceCount++;
	}
	

	// Create the Direct3D 11 API device object and a corresponding context.
	ID3D11DeviceContext*	pDeviceContext = NULL;
	ID3D11Device*			pDevice = NULL;

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	if (pAdapter)
	{
		driverType = D3D_DRIVER_TYPE_UNKNOWN;
	}
	hr = D3D11CreateDevice(
			pAdapter,// Specify nullptr to use the default adapter.
			driverType,
			NULL,
			creationFlags, // Set set debug and Direct2D compatibility flags.
			featureLevels, // List of feature levels this app can support.
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&pDevice, // Returns the Direct3D device created.
			&m_FeatureLevel, // Returns feature level of device created.
			&pDeviceContext // Returns the device immediate context.
			);
	if (pAdapter)
	{
		pAdapter->Release();
		pAdapter = NULL;
	}
	if (pFactory)
	{
		pFactory->Release();
		pFactory = NULL;
	}
		
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}
	

	hr = pDevice->QueryInterface(__uuidof(ID3D11Device1),(void**)&m_pD3DDevice);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}



	hr = pDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1),(void**)&m_pImmediateContext);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}
	SetFeatureLevel(m_FeatureLevel);

	pDeviceContext->Release();
	pDevice->Release();
}
void CD3DRenderer::CreateSwapChain(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel,UINT Width,UINT Height)
{
	HRESULT		hr;
	// Store the window bounds so the next time we get a SizeChanged event we can
	// avoid rebuilding everything if the size is identical.
	


#ifdef _DEBUG
	if (m_pSwapChain)
	{
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
		__debugbreak();
	}
#endif


	// Otherwise, create a new one using the same adapter as the existing Direct3D device.

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = Width; // Match the size of the window.
	swapChainDesc.Height = Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // This is the most common swap chain format.
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
	swapChainDesc.Flags = 0;

	IDXGIDevice1*	pdxgiDevice = NULL;
	hr = m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**) &pdxgiDevice);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}

	IDXGIAdapter*	pdxgiAdapter = NULL;
	pdxgiDevice->GetAdapter(&pdxgiAdapter);

	IDXGIFactory2*	pdxgiFactory = NULL;
	hr = pdxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**) &pdxgiFactory);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}
		
	if (swapChainPanel)
	{
		if (m_pSwapChain)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
			//g_pLog->Write(txt);
#endif
			__debugbreak();
		}

		// create new
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		hr = pdxgiFactory->CreateSwapChainForComposition(m_pD3DDevice, &swapChainDesc, nullptr, &m_pSwapChain);
		//dxgiFactory->CreateSwapChainForComposition(
		//		m_d3dDevice.Get(),
		//		&swapChainDesc,
		//		nullptr,
		//		&m_swapChain
		//		)
		//	);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
			//g_pLog->Write(txt);
#endif
			__debugbreak();
		}

		// set the swap chain on the SwapChainBackgroundPanel
		reinterpret_cast<IUnknown*>(swapChainPanel)->QueryInterface(__uuidof(ISwapChainPanelNative), (void**) &m_pSwapChainPanelNative);
		hr = m_pSwapChainPanelNative->SetSwapChain(m_pSwapChain);
		if (FAILED(hr))
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
			//g_pLog->Write(txt);
#endif
			__debugbreak();
		}		

	}
	else
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"CreateSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}

	// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
	// ensures that the application will only render after each VSync, minimizing power consumption.
	pdxgiDevice->SetMaximumFrameLatency(1);


	
	
	m_pSwapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY);

	SetCompositionScale(m_compositionScaleX,m_compositionScaleY);
	

	pdxgiFactory->Release();
	pdxgiAdapter->Release();
	pdxgiDevice->Release();

}
BOOL CD3DRenderer::CreateD2D(float dpi)
{
	// 1) d3d Device생성 , d3d Device Context 생성
	// 2) d3d Device , d3d Device Context로부터 d3d Device .1과 d3d Device Context .1을 얻는다.
	// 3) d3d로부터 dxgi인터페이스를 얻는다
	// 4) dxgi로부터 D2D.1 인터페이스 생성 
	// 5) d2d device로부터 d2d DeivceContext생성
	
	ID2D1Factory1*			pd2dFactory = NULL;

	HRESULT hr;

	D2D1_FACTORY_OPTIONS options;
    ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

	ULONG ref_count = 0;
	
	IDXGIDevice*	pdxgiDevice = NULL;
	m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice),(void**)&pdxgiDevice);
	
	// D2D 생성

	// D2D.1 인터페이스 생성
	
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory1),&options,(void**)&pd2dFactory);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}

	// D2D.1 Device생성, 앞에서 생성한 dxgi인터페이스가 필요하다
	hr = pd2dFactory->CreateDevice(pdxgiDevice,&m_pD2DDevice);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}

    hr = m_pD2DDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pD2DDeviceContext
            );

	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
		//g_pLog->Write(txt);
#endif
		__debugbreak();
	}
	

	pdxgiDevice->Release();
	pdxgiDevice = NULL;
	
	
	// factory도 필요없으니 Release()
	pd2dFactory->Release();
	pd2dFactory = NULL;
	
	
	CreateD2DTargetBuffer(dpi);

	return TRUE;
}
void CD3DRenderer::CreateD2DTargetBuffer(float dpi)
{
	if (m_pD2DTargetBitmap)
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//		g_pLog->Write(txt);
#endif
		__debugbreak();
	}

	/*
	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
			);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	DX::ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			)
		);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
*/

	HRESULT		hr = S_OK;
	IDXGISurface2*	pdxgiBackBuffer = NULL;
	m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pdxgiBackBuffer));

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpi,
		dpi
		);

	hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pdxgiBackBuffer, &bitmapProperties, &m_pD2DTargetBitmap);
	if (FAILED(hr))
	{
#ifdef _DEBUG
		WCHAR	txt[512] = {0};
		swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//		g_pLog->Write(txt);
#endif
		__debugbreak();
	}

	pdxgiBackBuffer->Release();
	pdxgiBackBuffer = NULL;


	m_pD2DDeviceContext->SetTarget(m_pD2DTargetBitmap);
	m_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

}

void CD3DRenderer::CleanupD2DTargetBuffer()
{
	ULONG	ref_count = 0;
	if (m_pD2DDeviceContext)
	{
		m_pD2DDeviceContext->SetTarget(NULL);
	}

	if (m_pD2DTargetBitmap)
	{
		ref_count = m_pD2DTargetBitmap->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}

		m_pD2DTargetBitmap = NULL;

	}
}
BOOL CD3DRenderer::CreateDWrite()
{
	IDWriteFactory2*	pDWriteFactory = NULL;
	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory2),(IUnknown**)&m_pDWriteFactory);
	if (FAILED(hr))
	{
		__debugbreak();
	}
	hr = m_pDWriteFactory->CreateTextFormat(
				L"Arial",                // Font family name.
				NULL,                       // Font collection (NULL sets it to use the system font collection).
				DWRITE_FONT_WEIGHT_REGULAR,	DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,
				18.0f,L"en-us",
				&m_pTextFormat);

	if (FAILED(hr))
	{
		__debugbreak();
	}

	//hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//if (S_OK != hr)
	//	__debugbreak();

 //   hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	//if (S_OK != hr)
	//	__debugbreak();

	hr = m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red),&m_pTextBrush);
	if (FAILED(hr))
	{
		__debugbreak();
	}
	//m_pD2DDeviceContext->DrawText(
	
	return TRUE;
}

void CD3DRenderer::CleanupDWrite()
{
	if (m_pTextBrush)
	{
		m_pTextBrush->Release();
		m_pTextBrush = NULL;
	}

	if (m_pTextFormat)
	{
		m_pTextFormat->Release();
		m_pTextFormat = NULL;
	}
	
	if (m_pDWriteFactory)
	{
		m_pDWriteFactory->Release();
		m_pDWriteFactory = NULL;
	}
}
void CD3DRenderer::SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();


	m_logicalSize = Windows::Foundation::Size(static_cast<float>(swapChainPanel->ActualWidth), static_cast<float>(swapChainPanel->ActualHeight));
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_compositionScaleX = swapChainPanel->CompositionScaleX;
	m_compositionScaleY = swapChainPanel->CompositionScaleY;
	m_dpi = currentDisplayInformation->LogicalDpi;
	//m_d2dContext->SetDpi(m_dpi, m_dpi);

	//CreateWindowSizeDependentResources();
}

void CD3DRenderer::SetCompositionScale(float fCompositionScaleX,float fCompositionScaley)
{
	// Setup inverse scale on the swap chain
	DXGI_MATRIX_3X2_F inverseScale = { 0 };
	float	comp_scale_x = fCompositionScaleX;
	float	comp_scale_y = fCompositionScaley;
	inverseScale._11 = 1.0f / comp_scale_x;
	inverseScale._22 = 1.0f / comp_scale_y;

	IDXGISwapChain2*	pSwapChain2 = NULL;
	m_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain2),(void**)&pSwapChain2);
	pSwapChain2->SetMatrixTransform(&inverseScale);
	pSwapChain2->Release();
	pSwapChain2 = NULL;

	
}


BOOL CD3DRenderer::CreateBackBuffer(UINT uiWidth,UINT uiHeight)
{
	BOOL	bResult = FALSE;

	// Create a render target view
    HRESULT	hr = S_FALSE;

	hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&m_pBackBuffer );
    if( FAILED( hr ) )
		goto lb_return;

    hr = m_pD3DDevice->CreateRenderTargetView( m_pBackBuffer, NULL, &m_pDiffuseRTV );
    
    if( FAILED( hr ) )
		goto lb_return;
	
	

	// Create depth stencil texture
    D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
    desc.Width = uiWidth;
    desc.Height = uiHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
		
	hr = m_pD3DDevice->CreateTexture2D( &desc, NULL, &m_pDepthStencil );
    if( FAILED( hr ) )
		goto lb_return;

	// Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = desc.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pD3DDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDSV );
    if( FAILED( hr ) )
		goto lb_return;
	
	m_pImmediateContext->OMSetRenderTargets(1, &m_pDiffuseRTV, m_pDSV );

	float	rgba[4] = {0.0f,0.0f,0.0f,1.0f};

	m_pImmediateContext->ClearRenderTargetView(m_pDiffuseRTV,rgba);
	

	m_pImmediateContext->ClearDepthStencilView(m_pDSV,D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f,0);

	m_dwWidth = uiWidth;
	m_dwHeight = uiHeight;
	

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)uiWidth;
    vp.Height = (FLOAT)uiHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
	
	m_vp = vp;
    m_pImmediateContext->RSSetViewports(1,&m_vp);

	bResult = TRUE;

lb_return:
	return bResult;

}

void CD3DRenderer::CleanupBackBuffer()
{
	ULONG	ref_count = 0;
	
	if (m_pDSV) 
	{
		m_pDSV->Release();
		m_pDSV = NULL;

	}
	if (m_pDepthStencil) 
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = NULL;
	}


	if (m_pDiffuseRTV)
	{
		m_pDiffuseRTV->Release();
		m_pDiffuseRTV = NULL;
	}

	if (m_pBackBuffer)
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
	}
	
}
void CD3DRenderer::CreateStates()
{
	HRESULT		hr;
	// 레스터라이즈 상태
	D3D11_RASTERIZER_DESC rasterDesc;
	SetDefaultReasterizeValue(&rasterDesc);

	struct RASTER_TYPE
	{
		D3D11_CULL_MODE cullMode;
		D3D11_FILL_MODE fillMode;
	};
	RASTER_TYPE	rasterTypelist[] =
	{
		D3D11_CULL_BACK , D3D11_FILL_SOLID,
		D3D11_CULL_BACK , D3D11_FILL_WIREFRAME,
		D3D11_CULL_NONE , D3D11_FILL_SOLID,
		D3D11_CULL_NONE , D3D11_FILL_WIREFRAME,
		D3D11_CULL_FRONT , D3D11_FILL_SOLID,
		D3D11_CULL_FRONT , D3D11_FILL_WIREFRAME
		
		
	};
	const	DWORD	dwRasterTypeNum = _countof(rasterTypelist);
	for (DWORD i=0; i<dwRasterTypeNum; i++)
	{
		DWORD	dwIndex = GetRasterTypeIndex(rasterTypelist[i].cullMode,rasterTypelist[i].fillMode);

		rasterDesc.CullMode = rasterTypelist[i].cullMode;
		rasterDesc.FillMode = rasterTypelist[i].fillMode;
		hr = m_pD3DDevice->CreateRasterizerState(&rasterDesc,m_ppRasterizeState+dwIndex);
		
		if (FAILED(hr))
			__debugbreak();
	}
	
		
	// 블랜딩 상태들
	D3D11_BLEND_DESC	blendDesc;
	SetDefaultBlendValue(&blendDesc);

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_pD3DDevice->CreateBlendState(&blendDesc,m_ppBlendState+BLEND_TYPE_TRANSP);

	if (FAILED(hr))
		__debugbreak();

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	hr = m_pD3DDevice->CreateBlendState(&blendDesc,m_ppBlendState+BLEND_TYPE_ADD);
	
	if (FAILED(hr))
		__debugbreak();

	// 컬러출력 안하는 블랜딩 상태
	SetBlendValueColorWriteDisable(&blendDesc);
	hr = m_pD3DDevice->CreateBlendState(&blendDesc,m_ppBlendState+BLEND_TYPE_NO_COLOR);
	
	if (FAILED(hr))
		__debugbreak();
	

	
	// 샘플러 스테이트
	D3D11_SAMPLER_DESC	samplerDesc;
	SetDefaultSamplerValue(&samplerDesc);

	struct SAMPLER_TYPE
	{
		D3D11_TEXTURE_ADDRESS_MODE	Address;
		D3D11_FILTER				Filter;
	};
	SAMPLER_TYPE	samplerTypelist[] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,D3D11_FILTER_ANISOTROPIC,

		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_CLAMP,D3D11_FILTER_ANISOTROPIC,

		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_MIRROR,D3D11_FILTER_ANISOTROPIC,
		
		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_BORDER,D3D11_FILTER_ANISOTROPIC
	};

	const	DWORD	dwSamplerTypeNum = _countof(samplerTypelist);

	for (DWORD i=0; i<dwSamplerTypeNum; i++)
	{
		samplerDesc.AddressU = samplerTypelist[i].Address;
		samplerDesc.AddressV = samplerTypelist[i].Address;
		samplerDesc.AddressW = samplerTypelist[i].Address;
		samplerDesc.Filter = samplerTypelist[i].Filter;

		hr = m_pD3DDevice->CreateSamplerState(&samplerDesc,m_ppSamplerState+i);

		if (FAILED(hr))
			__debugbreak();

	}
	D3D11_SAMPLER_DESC SamDescShad = 
	{
        D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
        0,//FLOAT MipLODBias;
        0,//UINT MaxAnisotropy;
        D3D11_COMPARISON_LESS , //D3D11_COMPARISON_FUNC ComparisonFunc;
        1.0,1.0,1.0,1.0,//FLOAT BorderColor[ 4 ];
        0,//FLOAT MinLOD;
        0//FLOAT MaxLOD;   
    };

}


BOOL CD3DRenderer::SetD3DDebugSetting()
{
	BOOL	bResult = FALSE;
	ID3D11Debug*		pDebug = NULL;
	ID3D11InfoQueue*	pInfoQueue = NULL;

	HRESULT	hr;
	hr = m_pD3DDevice->QueryInterface( __uuidof(ID3D11Debug), (void**)&pDebug );
	if (FAILED(hr))
		goto lb_return;

	
	hr = pDebug->QueryInterface( __uuidof(ID3D11InfoQueue), (void**)&pInfoQueue );
	if (FAILED(hr))
		goto lb_return;

	pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION,TRUE);
	pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR,TRUE);

	//D3D11 WARNING: ID3D11DeviceContext::DrawIndexed: The Pixel Shader expects a Render Target View bound to slot 1, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here. [ EXECUTION WARNING #3146081: DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET]

#ifdef DISABLE_D3D_WARNING
	D3D11_MESSAGE_SEVERITY hide [] =
	{
		D3D11_MESSAGE_SEVERITY_WARNING,
	 // Add more message IDs here as needed
	 };
 /*
			Category	D3D11_MESSAGE_CATEGORY_EXECUTION	D3D11_MESSAGE_CATEGORY
		Severity	D3D11_MESSAGE_SEVERITY_WARNING	D3D11_MESSAGE_SEVERITY
		ID	3146081	D3D11_MESSAGE_ID
+		pDescription	0x000000000c31ace0 "ID3D11DeviceContext::DrawIndexed: The Pixel Shader expects a Render Target View bound to slot 1, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here."	const char *
		DescriptionByteLength	353	unsigned __int64

		*/
	D3D11_INFO_QUEUE_FILTER filter;
	memset( &filter, 0, sizeof(filter) );
	filter.DenyList.NumSeverities = _countof(hide);
	filter.DenyList.pSeverityList = hide;
	pInfoQueue->AddStorageFilterEntries( &filter );
#endif

lb_return:
	if (pInfoQueue)
	{
		pInfoQueue->Release();
		pInfoQueue  = NULL;
	}
	if (pDebug)
	{
		pDebug->Release();
		pDebug = NULL;
	}
	
	return TRUE;
 
}

BOOL CD3DRenderer::CreateWritableTexture(DWORD dwWidth,DWORD dwHeight)
{
	BOOL	bResult = FALSE;
	DWORD*	pInitImage = new DWORD[dwWidth*dwHeight];
	memset(pInitImage,0,sizeof(DWORD)*dwWidth*dwHeight);

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_SUBRESOURCE_DATA	initData;
	initData.pSysMem = pInitImage;
	initData.SysMemPitch = dwWidth*sizeof(DWORD);
	initData.SysMemSlicePitch = 0;



	// YUV Texture
	bResult = CreateShaderResourceViewFromTex2D(&m_pYUVTextureSRV,&m_pYUVTexture,dwWidth,dwHeight,format,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE,&initData);
	if (bResult)
	{
		m_dwTextureWidth = dwWidth;
		m_dwTextureHeight = dwHeight;
	}
	
	
lb_return:
	delete [] pInitImage;
	return bResult;
}

void CD3DRenderer::DeleteWritableTexture()
{
	if (m_pYUVTexture)
	{
		m_pYUVTexture->Release();
		m_pYUVTexture = NULL;
	}
	if (m_pYUVTextureSRV)
	{
		m_pYUVTextureSRV->Release();
		m_pYUVTextureSRV = NULL;
	}

}
BOOL CD3DRenderer::CreateShaderResourceViewFromTex2D(ID3D11ShaderResourceView** ppOutSRV,ID3D11Texture2D** ppOutTexture,UINT Width,UINT Height,DXGI_FORMAT Format,D3D11_USAGE Usage,UINT BindFlags,UINT CPUAccessFlags,D3D11_SUBRESOURCE_DATA* pInitData)
{
	BOOL	bResult = FALSE;
		
	ID3D11Device*				pDevice = m_pD3DDevice;
	ID3D11ShaderResourceView*	pTexResource = NULL;
	
	
	D3D11_TEXTURE2D_DESC	desc;
	memset(&desc,0,sizeof(desc));
	desc.Width = Width;
	desc.Height = Height;
	desc.ArraySize = 1;
	desc.BindFlags = BindFlags;
	desc.CPUAccessFlags = CPUAccessFlags;
	desc.Format = Format;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = 0;
	desc.Usage = Usage;
	
	ID3D11Texture2D*	pTex2D = NULL;


	HRESULT hr = pDevice->CreateTexture2D(&desc,pInitData,&pTex2D);
	if (FAILED(hr))
	{
		OutputD3DErrorMsg(hr,L"Fail to CreateShaderResourceViewFromTex2D-CreateTexture2D");
		goto lb_return;
	}
	
	hr = pDevice->CreateShaderResourceView(pTex2D,NULL,&pTexResource);
	
	if (FAILED(hr))
	{
		OutputD3DErrorMsg(hr,L"Fail to CreateShaderResourceViewFromTex2D-CreateShaderResourceView");
		pTex2D->Release();
		pTex2D = NULL;

	}
	*ppOutTexture = pTex2D;
	*ppOutSRV = pTexResource;
	bResult = TRUE;
	
lb_return:
	return bResult;
}
BOOL CD3DRenderer::UpdateWindowSize(const Windows::Foundation::Size* pNewLogicalSize,float fCompositionScaleX,float fCompositionScaleY)
{
	BOOL	bResult = FALSE;
	
	
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

	CoreWindow^ window = CoreWindow::GetForCurrentThread();
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	if (m_logicalSize == *pNewLogicalSize && m_compositionScaleX == fCompositionScaleX && m_compositionScaleY == fCompositionScaleY)
		goto lb_return;
	
	m_logicalSize = *pNewLogicalSize;
	m_compositionScaleX = fCompositionScaleX;
	m_compositionScaleY = fCompositionScaleY;

	float	dpi = currentDisplayInformation->LogicalDpi;
	
	
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;


	
	m_pImmediateContext->OMSetRenderTargets(0, NULL, NULL);	
	
	CleanupD2DTargetBuffer();
	CleanupBackBuffer();

	m_outputSize.Width = m_logicalSize.Width * m_compositionScaleX;
	m_outputSize.Height = m_logicalSize.Height * m_compositionScaleY;
	
	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);

	m_d3dRenderTargetSize.Width = m_outputSize.Width;
	m_d3dRenderTargetSize.Height = m_outputSize.Height;
	
	UINT	Width = lround(m_d3dRenderTargetSize.Width);
	UINT	Height = lround(m_d3dRenderTargetSize.Height);

	DXGI_SWAP_CHAIN_DESC	desc;
	HRESULT	hr = m_pSwapChain->GetDesc(&desc);
	hr = m_pSwapChain->ResizeBuffers(desc.BufferCount,Width,Height, desc.BufferDesc.Format, 0);
	

	CreateBackBuffer(Width,Height);
	CreateD2DTargetBuffer(dpi);

	m_pSwapChain->SetRotation(DXGI_MODE_ROTATION_IDENTITY);
	SetCompositionScale(m_compositionScaleX,m_compositionScaleY);


	
	
	bResult = TRUE;
lb_return:
	return bResult;

}
/*
BOOL CD3DRenderer::UpdateWindowSize(UINT uiWidth,UINT uiHeight)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

	BOOL	bResult = FALSE;
	

	if (!(uiWidth * uiHeight))
		goto lb_return;


	if (m_dwWidth == (DWORD)uiWidth && m_dwHeight == (DWORD)uiHeight)
		goto lb_return;

	

	pDeviceContext->OMSetRenderTargets(0,NULL,NULL);

	if (m_pDSV)
	{
		m_pDSV->Release();
		m_pDSV = NULL;
	}
	if (m_pDepthStencil)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = NULL;
	}
	if (m_pDiffuseRTV)
	{
		m_pDiffuseRTV->Release();
		m_pDiffuseRTV = NULL;
	}
	if (m_pBackBuffer)
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
	}

	
	DXGI_SWAP_CHAIN_DESC	desc;
	HRESULT	hr = m_pSwapChain->GetDesc(&desc);

	if (FAILED(hr))
		__debugbreak();
	
	hr = m_pSwapChain->ResizeBuffers(desc.BufferCount,uiWidth,uiHeight,desc.BufferDesc.Format,0);
	
	if (FAILED(hr))
		__debugbreak();

	if (!CreateBackBuffer(uiWidth,uiHeight))
		goto lb_return;

	
	bResult = TRUE;

lb_return:

	return bResult;

}
*/
BOOL CD3DRenderer::InitBuffer()
{
	// create vertex buffer for position

	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_BUFFER_DESC bd;
	
	HRESULT hr;

	D3DTVERTEX	vertex[4] = {
		0.0f,1.0f,1.0f , 0.0f,1.0f, 
		1.0f,1.0f,1.0f , 1.0f,1.0f,
		1.0f,0.0f,1.0f , 1.0f,0.0f,
		0.0f,0.0f,1.0f , 0.0f,0.0f
	};

	ID3D11Device*	pDevice = m_pD3DDevice;
	
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertex;
		
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    bd.ByteWidth = sizeof(D3DTVERTEX)*4;

	hr = pDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		__debugbreak();

	// create index buffer
	WORD	index[6];
	index[0] = 0;
	index[1] = 3;
	index[2] = 2;

	index[3] = 0;
	index[4] = 2;
	index[5] = 1;
	
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = index;
	
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = sizeof(WORD)*6;
	
	hr = pDevice->CreateBuffer(&bd,&InitData,&m_pIndexBuffer);
	if (FAILED(hr))
		__debugbreak();


	// create constant buffer
	ZeroMemory( &bd, sizeof(bd) );

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    bd.ByteWidth = sizeof(CONSTANT_BUFFER_SPRITE);

	hr = pDevice->CreateBuffer( &bd, NULL, &m_pConstantBuffer );
	if (FAILED(hr))
		__debugbreak();

	return TRUE;
}
void CD3DRenderer::CleanupBuffer()
{
	if (m_pConstantBuffer)
	{
		m_pConstantBuffer->Release();
		m_pConstantBuffer = NULL;
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = NULL;
	}
}
BOOL CD3DRenderer::InitShader()
{
	BOOL	bResult = FALSE;

	HRESULT	hr = S_OK;
	

	m_pVS = CreateShader("./Shaders/sh_sprite.hlsl","vsDefault",SHADER_TYPE_VERTEX_SHADER,0);
	m_pPS[PIXEL_SHADER_FILTER_DEFAULT] = CreateShader("./Shaders/sh_sprite.hlsl","psDefault",SHADER_TYPE_PIXEL_SHADER,0);
	m_pPS[PIXEL_SHADER_FILTER_BLUR] = CreateShader("./Shaders/sh_sprite.hlsl","psBlur",SHADER_TYPE_PIXEL_SHADER,0);
	m_pPS[PIXEL_SHADER_FILTER_EDGE] = CreateShader("./Shaders/sh_sprite.hlsl","psEdge",SHADER_TYPE_PIXEL_SHADER,0);
	m_pPS[PIXEL_SHADER_FILTER_GREY] = CreateShader("./Shaders/sh_sprite.hlsl","psGrey",SHADER_TYPE_PIXEL_SHADER,0);

	// Define the input layout
    D3D11_INPUT_ELEMENT_DESC layoutSprite[] =
    {
        { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElementsSprite = ARRAYSIZE( layoutSprite );
	
	void*	pCodeBuffer = m_pVS->pCodeBuffer;
	DWORD	dwCodeSize = m_pVS->dwCodeSize;
		
	ID3D11Device*		pDevice = m_pD3DDevice;
	
	hr = pDevice->CreateInputLayout(layoutSprite, numElementsSprite,pCodeBuffer,dwCodeSize, &m_pVertexLayout );
	if (FAILED(hr))
		__debugbreak();
	
	
	bResult = TRUE;

lb_return:
	return bResult;
}

void CD3DRenderer::CleanupShader()
{
	if (m_pVS)
	{
		ReleaseShader(m_pVS);
		m_pVS = NULL;
	}
	for (DWORD i=0; i<MAX_PIXEL_SHADER_FILTER_NUM; i++)
	{
		if (m_pPS[i])
		{
			ReleaseShader(m_pPS[i]);
			m_pPS[i] = NULL;
		}
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
		m_pVertexLayout = NULL;
	}
}

SHADER_HANDLE* CD3DRenderer::CreateShader(char* szShaderFileName,char* szEntryName,SHADER_TYPE ShaderType,DWORD ShaderParams)
{
	BOOL				bResult = FALSE;

	SYSTEMTIME	CreationTime = {0};
	ID3D11Device*		pDevice = m_pD3DDevice;
	SHADER_HANDLE*		pNewShaderHandle = NULL;
	
	char	szPureFileName[_MAX_PATH];

	char	szShaderName[MAX_SHADER_NAME_BUFFER_LEN];
	
	DWORD	dwPureFileNameLen = GetNameRemovePath(szPureFileName,szShaderFileName);

	if (!dwPureFileNameLen)
		goto lb_return;
	
	CharToSmallASCII(szPureFileName,szPureFileName,dwPureFileNameLen);

	// 쉐이더 네임
	DWORD	dwShaderNameLen = sprintf_s(szShaderName,_countof(szShaderName),"%s_%s",szPureFileName,szEntryName);

	IUnknown*	pD3DShader = NULL;
	ID3DBlob*	pBlob = NULL;
	
	char*	szVSTarget = "vs_4_0";
	char*	szPSTarget = "ps_4_0";
	
	if (m_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		szVSTarget = "vs_4_0_level_9_3";
		szPSTarget = "ps_4_0_level_9_3";

		if (ShaderType > SHADER_TYPE_PIXEL_SHADER)
			goto lb_return;
	}

	switch (ShaderType)
	{
	case SHADER_TYPE_VERTEX_SHADER:
		{
			HRESULT	hr = CompileShaderFromFile(szShaderFileName,szEntryName, szVSTarget, &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			
			// Create the vertex shader
			hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11VertexShader**)&pD3DShader );
			if ( FAILED( hr ) )
			{	
				goto lb_exit;
			}
		}
		break;
	case SHADER_TYPE_PIXEL_SHADER:
		{
			HRESULT hr = CompileShaderFromFile(szShaderFileName,szEntryName, szPSTarget, &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11PixelShader**)&pD3DShader );
			if( FAILED( hr ) )
			{
				goto lb_exit;
			}
		}
		break;
	case SHADER_TYPE_HULL_SHADER:
		{
			HRESULT hr = CompileShaderFromFile(szShaderFileName,szEntryName, "hs_5_0", &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			hr = pDevice->CreateHullShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11HullShader**)&pD3DShader );
			if( FAILED( hr ) )
			{
				goto lb_exit;
			}
		}
		break;
		
	case SHADER_TYPE_DOMAIN_SHADER:
		{
			HRESULT hr = CompileShaderFromFile(szShaderFileName,szEntryName, "ds_5_0", &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			hr = pDevice->CreateDomainShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11DomainShader**)&pD3DShader);
			if( FAILED( hr ) )
			{
				goto lb_exit;
			}
		}
		break;

	case SHADER_TYPE_GEOMETRY_SHADER:
		{
			HRESULT hr = CompileShaderFromFile(szShaderFileName,szEntryName, "gs_4_0", &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			hr = pDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11GeometryShader**)&pD3DShader);
			if( FAILED( hr ) )
			{
				goto lb_exit;
			}
		}
		break;
	case SHADER_TYPE_COMPUTE_SHADER:
		{
			HRESULT hr = CompileShaderFromFile(szShaderFileName,szEntryName, "cs_5_0", &pBlob,ShaderParams,&CreationTime);
			if( FAILED( hr ) )
			{
				OutputFailToLoadShader(szShaderName);
				goto lb_exit;
			}
			hr = pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL,(ID3D11ComputeShader**)&pD3DShader);
			if( FAILED( hr ) )
			{
				goto lb_exit;
			}
		}
	}
	DWORD	dwCodeSize = (DWORD)pBlob->GetBufferSize();	
	char*	pCodeBuffer = (char*)pBlob->GetBufferPointer();


	// 중복되는 쉐이더코드는 없다. 따라서 새로 생성한다.
	
	pNewShaderHandle = CreateShaderHandle(szShaderName,dwShaderNameLen,szPureFileName,strlen(szPureFileName),&CreationTime,pCodeBuffer,dwCodeSize,ShaderType);
	
	if (pD3DShader)
	{
		pD3DShader->AddRef();
		pNewShaderHandle->pD3DShader = pD3DShader;
	}
	
lb_regist:
	pNewShaderHandle->dwRefCount++;

	bResult = TRUE;
	

lb_exit:
	if (pD3DShader)
	{
		pD3DShader->Release();
		pD3DShader = NULL;
	}
	if (pBlob)
	{
		pBlob->Release();
		pBlob = NULL;
	}
	
	
lb_return:
	return pNewShaderHandle;
}

SHADER_HANDLE* CD3DRenderer::CreateShaderHandle(char* szShaderName,DWORD dwShaderNameLen,char* szShaderFileName,DWORD dwShaderFileNameLen,SYSTEMTIME* pCreationTime,void* pCodeBuffer,DWORD dwCodeSize,SHADER_TYPE ShaderType)
{
	DWORD	ShaderHandleSize = sizeof(SHADER_HANDLE) - sizeof(DWORD) + dwCodeSize;
	SHADER_HANDLE*	pNewShaderHandle = (SHADER_HANDLE*)malloc(ShaderHandleSize);
	memset(pNewShaderHandle,0,ShaderHandleSize);
	
	// Shader Name
	memcpy(pNewShaderHandle->szShaderName,szShaderName,dwShaderNameLen);
	pNewShaderHandle->dwShaderNameLen = dwShaderNameLen;
	
	// Shader File Name
	memcpy(pNewShaderHandle->szShaderFileName,szShaderFileName,dwShaderFileNameLen);
	pNewShaderHandle->dwShaderFileNameLen = dwShaderFileNameLen;

	if (!memcmp(pNewShaderHandle->szShaderFileName,"shader",6))
		__debugbreak();

	// Creation Time
	pNewShaderHandle->CreationTime = *pCreationTime;
	

	pNewShaderHandle->dwCodeSize = dwCodeSize;
	memcpy(pNewShaderHandle->pCodeBuffer,pCodeBuffer,dwCodeSize);
	pNewShaderHandle->ShaderType = ShaderType;
	
	return pNewShaderHandle;
}
void CD3DRenderer::ReleaseShader(SHADER_HANDLE* pShaderHandle)
{
	pShaderHandle->dwRefCount--;
	
	if (pShaderHandle->dwRefCount)
		return;

	if (pShaderHandle->pD3DShader)
	{
		pShaderHandle->pD3DShader->Release();
		pShaderHandle->pD3DShader = NULL;
	}
		
	free(pShaderHandle);	
}

void CD3DRenderer::BeginRender(DWORD dwColor,DWORD dwFlags)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;

	COLOR_VALUE	backColor;
	backColor.r = (float)((dwColor & 0x00ff0000) >> 16) / 255.0f;	// R
	backColor.g = (float)((dwColor & 0x0000ff00) >> 8) / 255.0f;	// G
	backColor.b = (float)(dwColor & 0x000000ff) / 255.0f;			// B
	backColor.a = (float)((dwColor & 0xff000000) >> 24) / 255.0f;	// A
	backColor.a = 1.0f;

	// 렌더타겟 설정
	pDeviceContext->OMSetRenderTargets(1,&m_pDiffuseRTV,m_pDSV);
	pDeviceContext->RSSetViewports(1,&m_vp);	

	pDeviceContext->ClearRenderTargetView(m_pDiffuseRTV,backColor.rgba);
	pDeviceContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}
BOOL CD3DRenderer::Draw(DWORD dwWidth,DWORD dwHeight,DWORD dwPosX,DWORD dwPosY,DWORD dwColor,DWORD dwFlags)
{
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;
		
	
	CONSTANT_BUFFER_SPRITE		constBuffer;
	constBuffer.render_pos_x = (float)dwPosX;
	constBuffer.render_pos_y = (float)dwPosY;
	constBuffer.render_width = (float)dwWidth;
	constBuffer.render_height = (float)dwHeight;
	constBuffer.screen_width = (float)m_dwWidth;
	constBuffer.screen_height = (float)m_dwHeight;
	constBuffer.fAlpha = 1.0f;
	constBuffer.render_z = 0.5f;
	constBuffer.diffuseColor.Set(1.0f,1.0f,1.0f,1.0f);

	constBuffer.u_offset = 1.0f / (float)m_dwTextureWidth;
	constBuffer.v_offset = 1.0f / (float)m_dwTextureHeight;

	//ID3D11ShaderResourceView* pTexResource = m_pTextureSRV;
	
	
	
	
	ID3D11SamplerState*		pSamplerState = INL_GetSamplerState(SAMPLER_TYPE_WRAP_LINEAR);
	//pSamplerState = INL_GetSamplerState(SAMPLER_TYPE_WRAP_POINT);

	
	SHADER_HANDLE*		pVS = m_pVS;
	SHADER_HANDLE*		pPS = m_pPS[m_CurShaderFilter];
	
	//if (RENDER_TYPE_SPRITE_GRAY & dwFlags)
	//{
	//	__debugbreak();
	//	pPS = m_pPS_Grey;
	//}


	pDeviceContext->UpdateSubresource(m_pConstantBuffer,0,NULL,&constBuffer,0,0);
	
		
	// Set the input layout
    pDeviceContext->IASetInputLayout(m_pVertexLayout);

	// Set primitive topology
    pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// set vertex buffer
	
	UINT			Stride = sizeof(D3DTVERTEX);
	UINT			Offset = 0;
	
	pDeviceContext->IASetVertexBuffers(0,1,&m_pVertexBuffer,&Stride,&Offset);
	
	// set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	pDeviceContext->VSSetShader((ID3D11VertexShader*)pVS->pD3DShader,NULL,0);
	pDeviceContext->PSSetShader((ID3D11PixelShader*)pPS->pD3DShader,NULL,0);
		
	pDeviceContext->VSSetConstantBuffers(0,1,&m_pConstantBuffer);
	pDeviceContext->PSSetSamplers(0,1,&pSamplerState);

	

	ID3D11ShaderResourceView* pTexResource = m_pYUVTextureSRV;
	pDeviceContext->PSSetShaderResources(0,1,&pTexResource);	
	pDeviceContext->DrawIndexed(6,0,0);

	

	
	pTexResource = NULL;
	pDeviceContext->PSSetShaderResources(0,1,&pTexResource);	
	
	return TRUE;
}


BOOL CD3DRenderer::DrawText(const WCHAR* wchTxt, UINT32 cTexLen,D2D1_RECT_F* pRect,D2D1_COLOR_F& color)
{
	m_pD2DDeviceContext->BeginDraw();
	
	 
	m_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	//m_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Blue));


    // Use the DrawText method of the D2D render target interface to draw.
	//m_pTextBrush->SetColor(color);

	m_pD2DDeviceContext->DrawText(
        wchTxt,        // The string to render.
        cTexLen,    // The string's length.
        m_pTextFormat,    // The text format.
        pRect,       // The region of the window where the text will be rendered.
        m_pTextBrush     // The brush used to draw the text.
        );
	
	HRESULT hr = m_pD2DDeviceContext->EndDraw();
	return TRUE;
}

void CD3DRenderer::EndRender()
{
	
	COLOR_VALUE	cvBack;
	cvBack.Set(0.0f,1.0f,0.0f,0.01f);
	
	ID3D11DeviceContext*		pDeviceContext = m_pImmediateContext;
	
}
void CD3DRenderer::Present()
{
	UINT uiSyncInterval = 0;

	HRESULT hr = m_pSwapChain->Present(uiSyncInterval, 0);
	if (FAILED(hr))
	{

		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			__debugbreak();
			//HandleDeviceLost();
		}
		else
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
			//g_pLog->Write(txt);
#endif
			__debugbreak();
		}
	}
	
}

BOOL CD3DRenderer::SetPixelShaderFilter(PIXEL_SHADER_FILTER filter)
{
	BOOL	bResult = FALSE;

	if ((int)filter < 0 || (int)filter >= MAX_PIXEL_SHADER_FILTER_NUM)
		goto lb_return;

	m_CurShaderFilter = filter;
lb_return:
	return bResult;
}
BOOL CD3DRenderer::UpdateYUVTexture(DWORD dwWidth,DWORD dwHeight,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride)
{
	if (0 == dwWidth || 0 == dwHeight)
		__debugbreak();

	
	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		// 텍스쳐 다시 생성
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth,dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource,0,sizeof(mappedResource));
	
	HRESULT hr = pDeviceContext->Map(m_pYUVTexture,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);
	if (FAILED(hr))
		__debugbreak();
	
	const	BYTE*	y_buffer_entry = pYBuffer;
	const	BYTE*	u_buffer_entry = pUBuffer;
	const	BYTE*	v_buffer_entry = pVBuffer;

	DWORD	StrideHalf = Stride / 2;
	for (DWORD y=0; y<dwHeight; y++)
	{
		y_buffer_entry = pYBuffer + (Stride*y);
		u_buffer_entry = pUBuffer + (StrideHalf*(y>>1));
		v_buffer_entry = pVBuffer + (StrideHalf*(y>>1));

		BYTE*		pDestEntry = (BYTE*)mappedResource.pData + (mappedResource.RowPitch*y);

		for (DWORD x=0; x<dwWidth; x++)
		{
			pDestEntry[0] = *y_buffer_entry;
			pDestEntry[1] = *u_buffer_entry;
			pDestEntry[2] = *v_buffer_entry;
			pDestEntry[3] = 0xff;
			
			y_buffer_entry++;

			DWORD	uv_inc = x & 0x00000001;
			u_buffer_entry += uv_inc;
			v_buffer_entry += uv_inc;
			
			pDestEntry += 4;
		}
	}
	pDeviceContext->Unmap(m_pYUVTexture,0);

	return TRUE;
	
}
BOOL CD3DRenderer::UpdateYUVTexture10Bits(DWORD dwWidth,DWORD dwHeight,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride)
{
	if (0 == dwWidth || 0 == dwHeight)
		__debugbreak();

	
	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		// 텍스쳐 다시 생성
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth,dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource,0,sizeof(mappedResource));
	
	HRESULT hr = pDeviceContext->Map(m_pYUVTexture,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);
	if (FAILED(hr))
		__debugbreak();
	
	const WORD*	y_buffer_entry = NULL;
	const WORD*	u_buffer_entry = NULL;
	const WORD*	v_buffer_entry = NULL;

	DWORD	StrideHalf = Stride / 2;

	DWORD		x,y;
	BYTE*		pDestEntry = NULL;
	for (y=0; y<dwHeight; y++)
	{
		y_buffer_entry = (WORD*)(pYBuffer + Stride*y);
		u_buffer_entry = (WORD*)(pUBuffer + StrideHalf*(y>>1));
		v_buffer_entry = (WORD*)(pVBuffer + StrideHalf*(y>>1));

		pDestEntry = (BYTE*)mappedResource.pData + (mappedResource.RowPitch*y);

		for (x=0; x<dwWidth; x++)
		{
			pDestEntry[0] = (BYTE)(*y_buffer_entry>>2);
			pDestEntry[1] = (BYTE)(*u_buffer_entry>>2);
			pDestEntry[2] = (BYTE)(*v_buffer_entry>>2);
			pDestEntry[3] = 0xff;
			
			y_buffer_entry++;

			DWORD	uv_inc = x & 0x00000001;
			u_buffer_entry += uv_inc;
			v_buffer_entry += uv_inc;
			
			pDestEntry += 4;
		}
	}
	pDeviceContext->Unmap(m_pYUVTexture,0);

	return TRUE;
	
}
void CD3DRenderer::OutputFailToLoadShader(char* szUniqShaderName)
{
	WCHAR	wchTxt[128];
	swprintf_s(wchTxt,L"Fail to Load Shader:%S\n",szUniqShaderName);
	OutputDebugString(wchTxt);
}

void CD3DRenderer::CleanupSwapChain()
{

	ULONG	ref_count = 0;
	if (m_pSwapChainPanelNative)
	{
		m_pSwapChainPanelNative->SetSwapChain(NULL);
		ref_count = m_pSwapChainPanelNative->Release();
		m_pSwapChainPanelNative = NULL;
	}

	if (m_pSwapChain)
	{
		
		// 이 경우는 정상적으로 처리.
		// XAML UI와 함께 사용중이고 창이 Visibility상태라면 여기서 ref_count가 1 남는다.
		ref_count = m_pSwapChain->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"CleanupSwapChain() - File:%S , Line:%d \n",__FILE__,__LINE__);
//				g_pLog->Write(txt);
#endif
			// alt + f4를 눌러서 종료하면 deactivate이벤트가 발생하지 않으므로 ref_count가 1남는다.
			__debugbreak();
		}
		m_pSwapChain = NULL;
	}
}
void CD3DRenderer::CleanupD3D()
{
	ULONG	ref_count = 0;

	if (m_pImmediateContext)
	{
		ref_count = m_pImmediateContext->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}
		m_pImmediateContext = NULL;
	}
	if (m_pD3DDevice)
	{
		ref_count = m_pD3DDevice->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}
		m_pD3DDevice = NULL;
	}
	OutputDebugString(L"D3D Cleanup completed.\n");
}

void CD3DRenderer::CleanupD2D()
{
	ULONG	ref_count = 0;

	CleanupD2DTargetBuffer();
	
	if (m_pD2DDeviceContext)
	{
		ref_count = m_pD2DDeviceContext->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}
		m_pD2DDeviceContext = NULL;
	}
	
	if (m_pD2DDevice)
	{
		ref_count = m_pD2DDevice->Release();
		if (ref_count)
		{
#ifdef _DEBUG
			WCHAR	txt[512] = {0};
			swprintf_s(txt,L"File:%S , Line:%d \n",__FILE__,__LINE__);
//			g_pLog->Write(txt);
#endif
			__debugbreak();
		}
		m_pD2DDevice = NULL;
	}
}
void CD3DRenderer::Cleanup()
{
	CleanupShader();
	CleanupBuffer();

	DeleteWritableTexture();

	if (m_pImmediateContext)
	{
		m_pImmediateContext->OMSetRenderTargets(0,NULL,NULL);
		m_pImmediateContext->ClearState();
	}
	
	for (DWORD i=0; i<BLEND_TYPE_NUM; i++)
	{
		if (m_ppBlendState[i])
		{
			m_ppBlendState[i]->Release();
			m_ppBlendState[i] = NULL;
		}
	}
	for (DWORD i=0; i<MAX_RASTER_TYPE_NUM; i++)
	{
		if (m_ppRasterizeState[i])
		{
			m_ppRasterizeState[i]->Release();
			m_ppRasterizeState[i] = NULL;
		}
	}

	for (DWORD i=0; i<MAX_DEPTH_TYPE_NUM; i++)
	{
		if (m_ppDepthStencilState[i])
		{
			m_ppDepthStencilState[i]->Release();
			m_ppDepthStencilState[i] = NULL;
		}
	}
	for (DWORD i=0; i<MAX_SAMPLER_TYPE_NUM; i++)
	{
		if (m_ppSamplerState[i])
		{
			m_ppSamplerState[i]->Release();
			m_ppSamplerState[i] = NULL;
		}
	}
	CleanupDWrite();
	CleanupD2D();
	CleanupBackBuffer();
	CleanupSwapChain();
	CleanupD3D();

	
	
}


void CD3DRenderer::CleanupSwapChainNaviePanel()
{

	ULONG	ref_count = 0;
	if (m_pSwapChainPanelNative)
	{
		m_pSwapChainPanelNative->SetSwapChain(NULL);
		ref_count = m_pSwapChainPanelNative->Release();
		m_pSwapChainPanelNative = NULL;
	}
}
CD3DRenderer::~CD3DRenderer()
{
	Cleanup();
}

/*
BOOL CD3DRenderer::UpdateWritableTexture(BYTE* pSrc, DWORD dwWidth,DWORD dwHeight,DWORD dwPitch)
{
	__debugbreak();

	if (0 == dwHeight || 0 == dwHeight)
		__debugbreak();

	if (m_dwTextureWidth != dwWidth || m_dwTextureHeight != dwHeight)
	{
		DeleteWritableTexture();
		CreateWritableTexture(dwWidth,dwHeight);
	}

	ID3D11Device*			pDevice = m_pD3DDevice;
	ID3D11DeviceContext*	pDeviceContext = m_pImmediateContext;
	
	
	D3D11_TEXTURE2D_DESC ddsc;
	m_pTexture->GetDesc(&ddsc);
		

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	memset(&mappedResource,0,sizeof(mappedResource));
	
	HRESULT hr = pDeviceContext->Map(m_pTexture,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);
	if (FAILED(hr))
		__debugbreak();
	

	BYTE*		pDest = (BYTE*)mappedResource.pData;
	
	for (DWORD y=0; y<dwHeight; y++)
	{
		memcpy(pDest,pSrc,sizeof(DWORD)*dwWidth);
		pDest += mappedResource.RowPitch;
		pSrc += dwPitch;
	}
//	pOutLockedRect->pBits = pDest;
//	pOutLockedRect->Pitch = mappedResource.RowPitch;

	

	
	pDeviceContext->Unmap(m_pTexture,0);

	return TRUE;
	
}
*/