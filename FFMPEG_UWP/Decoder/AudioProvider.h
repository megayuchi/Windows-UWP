#pragma once


extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

using namespace Platform;
using namespace Windows::Foundation;

class CDecoder;
namespace MediaSourceProvider
{
	public ref class AudioProvider sealed
	{
		CDecoder*				m_pDecoder = nullptr;
		MediaStreamSource^		m_audioStreamSource = nullptr;
		Windows::Foundation::TimeSpan m_mediaDuration;

		int						m_iAudioStreamIndex = -1;
		AVCodecContext*			m_pACtx = NULL;
		AVFormatContext*		m_pFmtCtx = NULL;

		Windows::Foundation::EventRegistrationToken	m_startingRequestedToken;
		Windows::Foundation::EventRegistrationToken	m_sampleRequestedToken;
		
		void	FlushAudio();
		~AudioProvider();
		
	internal:
		void	Initialize(CDecoder* pDecoder,MediaStreamSource^ mss,Windows::Foundation::TimeSpan duration,int iAudioStreamIndex,AVFormatContext* pFmtCtx,AVCodecContext* pACtx);
		void	Cleanup();
	public:
		void OnStarting(MediaStreamSource ^sender, MediaStreamSourceStartingEventArgs ^args);
		void OnSampleRequested(MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args);
		MediaStreamSource^	GetAudioStreamSource()	{return m_audioStreamSource;}
		AudioProvider();
		
	};
}

