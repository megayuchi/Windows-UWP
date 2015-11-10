#include "pch.h"
#include "AudioProvider.h"
#include "Decoder.h"

using namespace Windows;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;


extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

#include "../Common_UTIL/debug_new.h"

using namespace MediaSourceProvider;

AudioProvider::AudioProvider()
{
}

void AudioProvider::OnStarting(MediaStreamSource ^sender, MediaStreamSourceStartingEventArgs ^args)
{
	MediaStreamSourceStartingRequest^ request = args->Request;

	// Perform seek operation when MediaStreamSource received seek event from MediaElement
	if (request->StartPosition && request->StartPosition->Value.Duration <= m_mediaDuration.Duration)
	{
		int streamIndex = m_iAudioStreamIndex;

		if (streamIndex >= 0)
		{
			
			// Convert TimeSpan unit to AV_TIME_BASE
			int64_t seekTarget = static_cast<int64_t>(request->StartPosition->Value.Duration / (av_q2d(m_pFmtCtx->streams[streamIndex]->time_base) * 10000000));

			if (av_seek_frame(m_pFmtCtx, streamIndex, seekTarget, 0) < 0)
			{
				// 아직 처리하지 않음.
				__debugbreak();
			}
			else
			{
				// Add deferral
				FlushAudio();
				avcodec_flush_buffers(m_pACtx);
			}
		}

		request->SetActualStartPosition(request->StartPosition->Value);
	}
}
void AudioProvider::OnSampleRequested(MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args)
{	

	MediaStreamSample^ sample = nullptr;
	
	
lb_try:
	BOOL	bDestroyed = FALSE;
	sample = m_pDecoder->GetNextAudioSample(&bDestroyed);
	
	if (!sample)
	{
		if (bDestroyed)
			goto lb_exit;

		Sleep(0);
		goto lb_try;
	}

	auto t = sample->Timestamp;
	__int64 time_stamp = t.Duration;
	
	
lb_exit:
	args->Request->Sample = sample;
	
}
void AudioProvider::FlushAudio()
{
	m_pDecoder->EmptyAudioQ();
}

void AudioProvider::Initialize(CDecoder* pDecoder,MediaStreamSource^ mss,Windows::Foundation::TimeSpan duration,int iAudioStreamIndex,AVFormatContext* pFmtCtx,AVCodecContext* pACtx)
{
	m_pDecoder = pDecoder;
	m_audioStreamSource = mss;
	m_mediaDuration = duration;
	m_iAudioStreamIndex = iAudioStreamIndex;
	m_pFmtCtx = pFmtCtx;
	m_pACtx = pACtx;

	m_audioStreamSource->Duration = duration;
	m_audioStreamSource->CanSeek = true;
	
	m_startingRequestedToken = m_audioStreamSource->Starting += ref new TypedEventHandler<MediaStreamSource ^, MediaStreamSourceStartingEventArgs ^>(this, &AudioProvider::OnStarting);
	m_sampleRequestedToken = m_audioStreamSource->SampleRequested += ref new TypedEventHandler<MediaStreamSource ^, MediaStreamSourceSampleRequestedEventArgs ^>(this, &AudioProvider::OnSampleRequested);
	
}

void AudioProvider::Cleanup()
{
	m_audioStreamSource->Starting -= m_startingRequestedToken;
	m_audioStreamSource->SampleRequested -= m_sampleRequestedToken;
	m_audioStreamSource = nullptr;


}
AudioProvider::~AudioProvider()
{
	Cleanup();
}


