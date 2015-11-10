#pragma once

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;

class CVideoPlayer
{
	CD3DRenderer*	m_pRenderer;
	IDecoder*		m_pDecoder;
	
	ULONGLONG	m_PrvProcessTick = 0;
	double		m_CurFrameSec = 0;
	BOOL		m_bGotFirstFrame = FALSE;
	
	DWORD		m_dwFPS;
	DWORD		m_dwFrameCount;
	ULONGLONG	m_ullPrvTickOnUpdate;
	DWORD		m_dwStatusTextLen;
	WCHAR		m_wchStatusText[128];
	BOOL		m_bCanRender = FALSE;
	MediaStreamSource^	m_audioStreamSource = nullptr;
	MediaElement^		m_mediaElement = nullptr;
	
	
	void	UpdateInfoTest(ULONGLONG CurTick);
	void	OnRender(__int64 time_stamp);
	void	Cleanup();
public:
	void	OnDecoderEvent(void* pData,DECODER_EVENT_TYPE type);
	void	OnTick(Windows::System::Threading::ThreadPoolTimer^ timer);
	BOOL	Initialize(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel);
	BOOL	Play(IStream* fileStreamData,MediaElement^ mediaElement);
	void	Stop();
	void	UpdateForWindowSizeChange(const Windows::Foundation::Size* pNewLogicalSize,float fCompositionScaleX,float fCompositionScaleY);
	void	Render();
	void	Process();
	void	CleanupSwapChainNaviePanel();
	
	BOOL	SetPixelShaderFilter(PIXEL_SHADER_FILTER filter);

	CVideoPlayer();
	~CVideoPlayer();
};

extern CVideoPlayer*	g_pVideoPlayer;
