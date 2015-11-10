#include "pch.h"
#include <Windows.h>
#include "D3DRenderer.h"
#include <ppltasks.h>
#include "VideoPlayer.h"

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace concurrency;

CVideoPlayer*	g_pVideoPlayer = NULL;

void __stdcall OnDecoderEvent(void* pData,DECODER_EVENT_TYPE type)
{
	CVideoPlayer*	pVideoPlayer = g_pVideoPlayer;
	pVideoPlayer->OnDecoderEvent(pData,type);
		
}



CVideoPlayer::CVideoPlayer()
{
	m_pRenderer = NULL;
	m_pDecoder = NULL;

	m_dwFPS = 0;
	m_dwFrameCount = 0;
	m_ullPrvTickOnUpdate = GetTickCount64();
	m_dwStatusTextLen = 0;
	memset(m_wchStatusText,0,sizeof(m_wchStatusText));

	g_pVideoPlayer = this;
}

HRESULT typedef (__stdcall *CREATE_INSTANCE_FUNC)(void* ppv);

BOOL CVideoPlayer::Initialize(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel)
{
	BOOL	bResult = FALSE;

	CPU_ARCH		arch = CPU_ARCH_X86;
	BUILD_CONFIG	config = BUILD_CONFIG_DEBUG;
	// OS
		
	// Architecture
#if defined(ARM_ARCH)
	arch = CPU_ARCH_ARM;
#elif defined(WIN64)
	arch = CPU_ARCH_X64;
#else
	arch = CPU_ARCH_X86;
#endif

	// debug / release
#ifdef _DEBUG
	config = BUILD_CONFIG_DEBUG;
#else
	config = BUILD_CONFIG_RELEASE;
#endif
	WCHAR		wchDocderDllFileName[_MAX_PATH];
	DWORD	dwLen = CreateDllFileName(wchDocderDllFileName,_countof(wchDocderDllFileName),L"Decoder",arch,config);

	HMODULE	hDecoderDLL = LoadPackagedLibrary(wchDocderDllFileName,0);
	if (!hDecoderDLL)
		goto lb_return;


	CREATE_INSTANCE_FUNC	pFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(hDecoderDLL,"DllCreateInstance");
	if (!pFunc)
		goto lb_return;

	HRESULT	hr = pFunc(&m_pDecoder);
	if (S_OK != hr)
		goto lb_return;

	m_pDecoder->Initialize(::OnDecoderEvent);

	m_pRenderer = new CD3DRenderer;
	m_pRenderer->Initialize(swapChainPanel,0);

		

	bResult = TRUE;

lb_return:
	return bResult;
	
}
void CVideoPlayer::OnDecoderEvent(void* pData,DECODER_EVENT_TYPE type)
{
	switch (type)
	{
	case DECODER_EVENT_TYPE_START:
		break;
	case DECODER_EVENT_TYPE_FIRST_FRAME_DECODED:
		m_bCanRender = TRUE;
		break;
	};
}


BOOL CVideoPlayer::Play(IStream* fileStreamData,MediaElement^ mediaElement)
{
	Stop();

	m_bGotFirstFrame = FALSE;
	m_PrvProcessTick = GetTickCount64();

	BOOL	bResult = m_pDecoder->Decode(fileStreamData);
	
	m_audioStreamSource = m_pDecoder->GetAudioStreamSource();
	if (m_audioStreamSource)
	{
		IUnknown*	pObj = reinterpret_cast<IUnknown*>(m_audioStreamSource);
		ULONG ref_count = GetRefCount(pObj);
	}
	m_mediaElement = mediaElement;
	m_mediaElement->SetMediaStreamSource(m_audioStreamSource);
	m_mediaElement->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	
	return bResult;
}

void CVideoPlayer::OnTick(Windows::System::Threading::ThreadPoolTimer^ timer)
{
	auto coreDispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;
	coreDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		Process();
	}));
}
void CVideoPlayer::Stop()
{
	if (m_mediaElement)
	{
		m_mediaElement->Stop();
		m_mediaElement = nullptr;
	}

	m_audioStreamSource = nullptr;
	m_pDecoder->Stop();
	m_bCanRender = FALSE;
}
void CVideoPlayer::UpdateForWindowSizeChange(const Windows::Foundation::Size* pNewLogicalSize,float fCompositionScaleX,float fCompositionScaleY)
{
	m_pRenderer->UpdateWindowSize(pNewLogicalSize,fCompositionScaleX,fCompositionScaleY);
}


void CVideoPlayer::Process()
{
	if (!m_pDecoder)
		return;

	if (!m_bCanRender)
		return;

	auto t = m_mediaElement->Position;
	__int64	time_stamp = t.Duration;

	OnRender(time_stamp);
	
}

void CVideoPlayer::OnRender(__int64 time_stamp)
{
	ULONGLONG	CurTick = GetTickCount64();
	
	DWORD	dwImageWidth = 0;
	DWORD	dwImageHeight = 0;
	DWORD	dwImageStride = 0;

	BYTE*	pYBuffer = NULL;
	BYTE*	pUBuffer = NULL;
	BYTE*	pVBuffer = NULL;
		
	DWORD	dwScreenWidth = m_pRenderer->GetWidth();
	DWORD	dwScreenHeight = m_pRenderer->GetHeight();

	DWORD	dwRenderWidth = dwScreenWidth;
	DWORD	dwRenderHeight = dwScreenHeight;

	ULONGLONG	ElapsedTick = CurTick - m_PrvProcessTick;
	
	

	YUV_FRAME* pYUVFrame = NULL;
	
	BOOL	bGotFrame = m_pDecoder->AcquireYUVFrameWithTimeStamp(&pYUVFrame,time_stamp);
	if (bGotFrame)
	{
		dwImageWidth = pYUVFrame->Width;
		dwImageHeight = pYUVFrame->Height;
		dwImageStride = pYUVFrame->Stride;
		pYBuffer = pYUVFrame->pYBuffer;
		pUBuffer = pYUVFrame->pUBuffer;
		pVBuffer = pYUVFrame->pVBuffer;

		m_bGotFirstFrame = TRUE;
			
		float aspect = (float)dwImageHeight / (float)dwImageWidth;

		dwRenderWidth = dwScreenWidth;
		dwRenderHeight = (DWORD)( (float)dwRenderWidth * aspect );

		if (dwRenderHeight > dwScreenHeight)
		{
			dwRenderHeight = dwScreenHeight;
			dwRenderWidth = (DWORD)( (float)dwRenderHeight * (1.0f / aspect) );
		}
	
		if (dwImageStride <= dwImageWidth)
		{
			m_pRenderer->UpdateYUVTexture(dwImageWidth,dwImageHeight,pYBuffer,pUBuffer,pVBuffer,dwImageStride);
		}
		else
		{
			m_pRenderer->UpdateYUVTexture10Bits(dwImageWidth,dwImageHeight,pYBuffer,pUBuffer,pVBuffer,dwImageStride);
		}



		m_pRenderer->BeginRender(0xff0000ff,0);
		m_pRenderer->Draw(dwRenderWidth,dwRenderHeight,0,0,0xff00ff00,0);
		m_pRenderer->EndRender();
	
		UpdateInfoTest(CurTick);
		m_pRenderer->Present();
	}
	else
	{
		int a = 0;
	}
lb_return:
	
	m_PrvProcessTick = CurTick;
}
void CVideoPlayer::UpdateInfoTest(ULONGLONG CurTick)
{
	// 프레임 레이트 갱신
	m_dwFrameCount++;
	if (CurTick - m_ullPrvTickOnUpdate > 1000)
	{
		m_dwFPS = m_dwFrameCount;
		m_dwFrameCount = 0;
		m_ullPrvTickOnUpdate = CurTick;

		Windows::System::AppMemoryReport^ amr = Windows::System::MemoryManager::GetAppMemoryReport();
		UINT64	MemUsage = (UINT64)amr->PrivateCommitUsage;
		UINT64	PeakMemUsage = (UINT64)amr->PeakPrivateCommitUsage;

		WCHAR	wchMemUsage[16];
		WCHAR	wchMemPeak[16];

		GetSizeText(wchMemUsage,_countof(wchMemUsage),MemUsage);
		GetSizeText(wchMemPeak,_countof(wchMemPeak),PeakMemUsage);

		m_dwStatusTextLen = swprintf_s(m_wchStatusText,L"%u FPS, MemUsage:%s , MemPeak:%s",m_dwFPS,wchMemUsage,wchMemPeak);
	}
	
	D2D1_RECT_F	rect = {10.0f,30.0f,250.0f,60.0f};
	m_pRenderer->DrawText(m_wchStatusText,m_dwStatusTextLen,&rect,D2D1::ColorF(D2D1::ColorF::Red));
}
void CVideoPlayer::Render()
{
	DWORD	dwScreenWidth = m_pRenderer->GetWidth();
	DWORD	dwScreenHeight = m_pRenderer->GetHeight();

	m_pRenderer->BeginRender(0xff0000ff,0);
	m_pRenderer->Draw(dwScreenWidth,dwScreenHeight,0,0,0xffffffff,0);
	m_pRenderer->EndRender();
	m_pRenderer->Present();
}
void CVideoPlayer::Cleanup()
{
	if (m_mediaElement)
	{
		m_mediaElement->Stop();
		
	}
	if (m_pRenderer)
	{
		delete m_pRenderer;
		m_pRenderer = NULL;
	}
	if (m_pDecoder)
	{
		m_pDecoder->Release();
		m_pDecoder = NULL;
	}
}
void CVideoPlayer::CleanupSwapChainNaviePanel()
{
	if (m_pRenderer)
	{
		m_pRenderer->CleanupSwapChainNaviePanel();
	}
}

BOOL CVideoPlayer::SetPixelShaderFilter(PIXEL_SHADER_FILTER filter)
{
	BOOL	bResult = FALSE;
	if (!m_pRenderer)
		goto lb_return;
	
	m_pRenderer->SetPixelShaderFilter(filter);

lb_return:
	return bResult;

}
CVideoPlayer::~CVideoPlayer()
{
	Cleanup();

}
