#pragma once

#include "d3d_typede.h"

enum PIXEL_SHADER_FILTER
{
	PIXEL_SHADER_FILTER_DEFAULT = 0,
	PIXEL_SHADER_FILTER_BLUR = 1,
	PIXEL_SHADER_FILTER_EDGE = 2,
	PIXEL_SHADER_FILTER_GREY = 3
};
#define MAX_PIXEL_SHADER_FILTER_NUM		4


class CD3DRenderer
{
	Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
	Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
	Windows::Foundation::Size						m_d3dRenderTargetSize;
	Windows::Foundation::Size						m_outputSize;
	Windows::Foundation::Size						m_logicalSize;
	float	m_compositionScaleX;
	float	m_compositionScaleY;
	float	m_dpi;

	DWORD	m_dwWidth;
	DWORD	m_dwHeight;
	DWORD	m_dwCreateFlags;

	D3D11_FILL_MODE				m_FillMode;
	D3D_FEATURE_LEVEL			m_FeatureLevel;
	D3D_DRIVER_TYPE				m_driverType;
	ISwapChainPanelNative*		m_pSwapChainPanelNative;
	ID3D11Device*				m_pD3DDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	
	ID2D1Device*				m_pD2DDevice;
	ID2D1DeviceContext*			m_pD2DDeviceContext;
	ID2D1Bitmap1*				m_pD2DTargetBitmap;

	IDWriteFactory2*			m_pDWriteFactory;
	ID2D1SolidColorBrush*		m_pTextBrush;
	IDWriteTextFormat*			m_pTextFormat;
	

	IDXGISwapChain1*			m_pSwapChain;
	ID3D11Texture2D*			m_pBackBuffer;
	ID3D11RenderTargetView*		m_pDiffuseRTV;
	ID3D11DepthStencilView*		m_pDSV;
	ID3D11Texture2D*			m_pDepthStencil;
	
	ID3D11DepthStencilState*	m_ppDepthStencilState[MAX_DEPTH_TYPE_NUM];
	ID3D11BlendState*			m_ppBlendState[BLEND_TYPE_NUM];
	ID3D11RasterizerState*		m_ppRasterizeState[MAX_RASTER_TYPE_NUM];
	ID3D11SamplerState*			m_ppSamplerState[MAX_SAMPLER_TYPE_NUM];
	D3D11_VIEWPORT				m_vp;

	DWORD						m_dwTextureWidth;
	DWORD						m_dwTextureHeight;
	ID3D11Texture2D*			m_pYUVTexture;
	ID3D11ShaderResourceView*	m_pYUVTextureSRV;

	ID3D11Buffer*				m_pVertexBuffer;
	ID3D11Buffer*				m_pIndexBuffer;
	ID3D11Buffer*				m_pConstantBuffer;
	ID3D11InputLayout*			m_pVertexLayout;
	SHADER_HANDLE*				m_pVS;
	SHADER_HANDLE*				m_pPS[MAX_PIXEL_SHADER_FILTER_NUM];
	
	PIXEL_SHADER_FILTER			m_CurShaderFilter;
	
	
	void	SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel);
	void	SetCompositionScale(float fCompositionScaleX,float fCompositionScaley);
	

	void	CreateStates();
	BOOL	CreateShaderResourceViewFromTex2D(ID3D11ShaderResourceView** ppOutSRV,ID3D11Texture2D** ppOutTexture,UINT Width,UINT Height,DXGI_FORMAT Format,D3D11_USAGE Usage,UINT BindFlags,UINT CPUAccessFlags,D3D11_SUBRESOURCE_DATA* pInitData);

	SHADER_HANDLE*	CreateShader(char* szShaderFileName,char* szEntryName,SHADER_TYPE ShaderType,DWORD ShaderParams);
	SHADER_HANDLE*	CreateShaderHandle(char* szShaderName,DWORD dwShaderNameLen,char* szShaderFileName,DWORD dwShaderFileNameLen,SYSTEMTIME* pCreationTime,void* pCodeBuffer,DWORD dwCodeSize,SHADER_TYPE ShaderType);
	void			ReleaseShader(SHADER_HANDLE* pShaderHandle);
	void			OutputFailToLoadShader(char* szUniqShaderName);

	

	BOOL	CreateBackBuffer(UINT uiWidth,UINT uiHeight);
	void	CleanupBackBuffer();

	void	CreateD3D();
	void	CleanupD3D();

	BOOL	CreateD2D(float dpi);
	void	CleanupD2D();

	void	CreateD2DTargetBuffer(float dpi);
	void	CleanupD2DTargetBuffer();

	BOOL	CreateDWrite();
	void	CleanupDWrite();

	void	CreateSwapChain(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel,UINT Width,UINT Height);
	void	CleanupSwapChain();

	BOOL	InitShader();
	void	CleanupShader();
	
	BOOL	InitBuffer();
	void	CleanupBuffer();

	BOOL	SetD3DDebugSetting();


	
	void	Cleanup();
public:
	BOOL CD3DRenderer::Initialize(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel,DWORD dwFlags);
	
	BOOL	CreateWritableTexture(DWORD dwWidth,DWORD dwHeight);
	void	DeleteWritableTexture();
	BOOL	UpdateYUVTexture(DWORD dwWidth,DWORD dwHeight,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride);
	BOOL	UpdateYUVTexture10Bits(DWORD dwWidth,DWORD dwHeight,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride);
	void	CleanupSwapChainNaviePanel();

	void	BeginRender(DWORD dwColor,DWORD dwFlags);
	BOOL	Draw(DWORD dwWidth,DWORD dwHeight,DWORD dwPosX,DWORD dwPosY,DWORD dwColor,DWORD dwFlags);
	BOOL	DrawText(const WCHAR* wchTxt, UINT32 cTexLen,D2D1_RECT_F* pRect,D2D1_COLOR_F& color);
	void	EndRender();
	void	Present();

	DWORD	GetWidth()	{return m_dwWidth;}
	DWORD	GetHeight()	{return m_dwHeight;}
	void	GetTextureSize(DWORD* pdwOutWidth,DWORD* pdwOutHeight)	{*pdwOutWidth = m_dwTextureWidth; *pdwOutHeight = m_dwTextureHeight;}
	BOOL	UpdateWindowSize(const Windows::Foundation::Size* pNewLogicalSize,float fCompositionScaleX,float fCompositionScaleY);
	BOOL	SetPixelShaderFilter(PIXEL_SHADER_FILTER filter);
	
	ID3D11SamplerState*		INL_GetSamplerState(SAMPLER_TYPE type)	{return m_ppSamplerState[type];}
	CD3DRenderer();
	~CD3DRenderer();
};

