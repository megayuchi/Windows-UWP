#include "pch.h"
#include <process.h>
#include <intrin.h>
#include "AudioProvider.h"


extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
}

#include "Util.h"
#include "Decoder.h"
#include "DecoderThread.h"
#include "CustomStreamFunc.h"
#include "YUVQueue.h"

#include "../Common_UTIL/debug_new.h"


///> Library Link On Windows System
#pragma comment( lib, "avformat.lib" )	
#pragma comment( lib, "avutil.lib" )
#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "swscale.lib" )
#pragma comment( lib, "swresample.lib" )

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

using namespace Windows;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;

using namespace MediaSourceProvider;

CDecoder::CDecoder()
{
#ifdef _DEBUG
	int	flag = _CRTDBG_ALLOC_MEM_DF |_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif

	m_dwRefCount = 1;
	m_hDecodeThread = NULL;

	m_lCanceled = 0;

	m_pFileStream = NULL;

	InitializeSRWLock(&m_LockVidoPacketQ);

}
STDMETHODIMP CDecoder::QueryInterface(REFIID refiid, void** ppv)
{
	*ppv = NULL;
	
	return E_NOINTERFACE;	
}
STDMETHODIMP_(ULONG) CDecoder::AddRef(void)
{
	m_dwRefCount++;
	return m_dwRefCount;
}
STDMETHODIMP_(ULONG) CDecoder::Release(void)
{
	DWORD	ref_count = --m_dwRefCount;
	if (!m_dwRefCount)
		delete this;

	return ref_count;
}
BOOL __stdcall CDecoder::Initialize(OnDecoderCallback pFunc)
{
	m_pOnDecoderEventFunc = pFunc;
	
	av_register_all();
	avformat_network_init();

	return TRUE;
}

BOOL __stdcall CDecoder::Decode(IStream* pFileStream)
{
	BOOL	bResult = FALSE;

	Stop();
		
	m_lCanceled = 0;

	if (pFileStream)
	{
		m_pFileStream = pFileStream;
		
	}
	else
	{
		__debugbreak();
	}

	
	
	if (!InitFFMpegStream())
	{
		CleanupFFMpegStream();
		goto lb_return;
	}
	
	CreateAudioQ();	
	CreateVideoQ();

	UINT	uiThreadID = -1;
	m_hDecodeThread = (HANDLE)_beginthreadex(NULL,0,DecodeThread,this,0,&uiThreadID);

	bResult = TRUE;
lb_return:
	return bResult;
}
BOOL CDecoder::InitFFMpegStream()
{
	BOOL	bResult = FALSE;

	WCHAR	wchTxt[256] = {0};
		
	int ret = -1;
		
	BYTE*	pSampleBuffer = NULL;
	
	//ret = avformat_open_input( &m_pFmtCtx, m_szCurPlayFileName, NULL, NULL );
	int sample_buffer_size = 16384;//
	pSampleBuffer = (BYTE*)av_malloc(sample_buffer_size);


	//AVInputFormat* inputFormat = av_find_input_format("mov,mp4,m4a,3gp,3g2,mj2,mkv");

	m_pIOCtx = avio_alloc_context(pSampleBuffer, sample_buffer_size,  // internal Buffer and its size
		0,                  // bWriteable (1=true,0=false) 
		m_pFileStream,      // user data ; will be passed to our callback functions
		ReadStreamFunc,
		0,                  // Write callback function (not used in this example) 
		SeekStreamFunc);

	m_pFmtCtx = avformat_alloc_context();

	m_pFmtCtx->pb = m_pIOCtx;
	m_pFmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

	ret = avformat_open_input(&m_pFmtCtx, "", 0, 0);
		
	if (0 != ret)
	{
		swprintf_s(wchTxt,L"File to Open (ret: %d)\n", ret);
		OutputDebugStringW(wchTxt);
#ifdef _DEBUG
		__debugbreak();
#endif
		goto lb_return;
	}
	swprintf_s(wchTxt,L"Format: %S\n", m_pFmtCtx->iformat->name );
	OutputDebugStringW(wchTxt);

	ret = avformat_find_stream_info( m_pFmtCtx, NULL );
	if (ret < 0) 
	{
		swprintf_s(wchTxt,L"Fail to get Stream Information\n" );
		OutputDebugStringW(wchTxt);
#ifdef _DEBUG
		__debugbreak();
#endif
		goto lb_return;
		
	}
	OutputDebugStringW(L"Get Stream Information Success\n" );
	
	for (unsigned int i=0 ; i < m_pFmtCtx->nb_streams ; i++ ) 
	{
		if (m_nVSI < 0 && m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			m_nVSI = i;
		}
		else if (m_nASI < 0 && m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
		{
			m_nASI = i;
		}
	}

	if (m_nVSI < 0 && m_nASI < 0) 
	{
		OutputDebugStringW(L"No Video & Audio Streams were Found\n");
#ifdef _DEBUG
		__debugbreak();
#endif
		goto lb_return;
	}
	
	// Fine Video Decoder
	if (m_nVSI >= 0)
	{
		m_pVCtx = m_pFmtCtx->streams[m_nVSI]->codec;
		AVCodec *pVideoCodec = avcodec_find_decoder(m_pVCtx->codec_id);
		if (pVideoCodec == NULL)
		{
			OutputDebugStringW(L"No Video Decoder was Found\n");
#ifdef _DEBUG
			__debugbreak();
#endif
			goto lb_return;

		}

		if (avcodec_open2(m_pVCtx, pVideoCodec, NULL) < 0)
		{
			OutputDebugStringW(L"Fail to Initialize Video Decoder\n");
#ifdef _DEBUG
			__debugbreak();
#endif
			goto lb_return;

		}
		//AV_PIX_FMT_YUV420P
		
		m_pVFrame = av_frame_alloc();

#ifdef _DEBUG
		if (AV_PIX_FMT_YUV420P == m_pVCtx->pix_fmt)
		{
			int a = 0;
		}
		else if (AV_PIX_FMT_YUV420P10LE == m_pVCtx->pix_fmt)
		{
			int a = 0;
		}
		else
		{
			int a = 0;
		}
#endif
		
	}

	// Fine Audio Decoder
	if (m_nASI >= 0)
	{
		m_pACtx = m_pFmtCtx->streams[m_nASI]->codec;

		AVCodec *pAudioCodec = avcodec_find_decoder(m_pACtx->codec_id);
		if (pAudioCodec == NULL)
		{
			OutputDebugStringW(L"Audio Decoder was not found\n");
#ifdef _DEBUG
			__debugbreak();
#endif
			goto lb_return;

		}
		///> Initialize Codec Context as Decoder
		if (avcodec_open2(m_pACtx, pAudioCodec, NULL ) < 0) 
		{
			OutputDebugStringW(L"Fail to Initialize Audio Decoder\n");
#ifdef _DEBUG
			__debugbreak();
#endif
			goto lb_return;
		}
		
		
		AudioStreamDescriptor^	audioStreamDescriptor = CreateAudioStreamDescriptor(m_pACtx,true);
		Windows::Foundation::TimeSpan mediaDuration = { LONGLONG(m_pFmtCtx->duration * 10000000 / double(AV_TIME_BASE)) };

		if (!audioStreamDescriptor)
			__debugbreak();

		MediaStreamSource^	audioStreamSource = ref new MediaStreamSource(audioStreamDescriptor);
		if (!audioStreamSource)
			__debugbreak();


		m_audioProvider = ref new AudioProvider;
		m_audioProvider->Initialize(this,audioStreamSource,mediaDuration,m_nASI,m_pFmtCtx,m_pACtx);

		audioStreamDescriptor = nullptr;
		audioStreamSource = nullptr;

		
		m_pAFrame = av_frame_alloc();
	}
	// Set default channel layout when the value is unknown (0)
	int64 inChannelLayout = m_pACtx->channel_layout ? m_pACtx->channel_layout : av_get_default_channel_layout(m_pACtx->channels);
	int64 outChannelLayout = av_get_default_channel_layout(m_pACtx->channels);

	// Set up resampler to convert any PCM format (e.g. AV_SAMPLE_FMT_FLTP) to AV_SAMPLE_FMT_S16 PCM format that is expected by Media Element.
	// Additional logic can be added to avoid resampling PCM data that is already in AV_SAMPLE_FMT_S16_PCM.
	m_pSwrCtx = swr_alloc_set_opts(
			NULL,
			outChannelLayout,
			AV_SAMPLE_FMT_S16,
			m_pACtx->sample_rate,
			inChannelLayout,
			m_pACtx->sample_fmt,
			m_pACtx->sample_rate,
			0,
			NULL);

		
	if (!m_pSwrCtx)
		__debugbreak();
	
	if (swr_init(m_pSwrCtx) < 0)
		__debugbreak();
	


	OutputDebugStringW(L"Begin decoding\n");

	m_pOnDecoderEventFunc((IDecoder*)this,DECODER_EVENT_TYPE_START);

	bResult = TRUE;
lb_return:
	return bResult;

}

BOOL CDecoder::Read()
{
	BOOL	bResult = FALSE;
	
	if (IsDecodeCanceled())
	{
		goto lb_return;
	}

	AVPacket avPacket;
	av_init_packet(&avPacket);
	avPacket.data = NULL;
	avPacket.size = 0;

	if (av_read_frame(m_pFmtCtx, &avPacket) < 0)
	{
		goto lb_return;
	}

	///> Decoding
	if (avPacket.stream_index == m_nVSI)
	{
		// 비디오의 경우 큐에 넣어두고 decode스레드에서 디코딩하게 한다.
		AcquireSRWLockExclusive(&m_LockVidoPacketQ);
		m_pVideoPacketQ->push(avPacket);
		ReleaseSRWLockExclusive(&m_LockVidoPacketQ);
		bResult = TRUE;
	}
	else if (avPacket.stream_index == m_nASI)
	{

		// 오디오의 경우 바로 디코딩
		while (avPacket.size > 0)
		{
			BOOL	bGotSound = FALSE;
			int decodedBytes = avcodec_decode_audio4(m_pACtx, m_pAFrame, &bGotSound, &avPacket);
			if (decodedBytes < 0)
			{
				break; // Skip broken frame
			}
			if (bGotSound)
			{
				// Resample uncompressed frame to AV_SAMPLE_FMT_S16 PCM format that is expected by Media Element
				uint8_t* resampledData = nullptr;
				unsigned int aBufferSize = av_samples_alloc(&resampledData, NULL, m_pAFrame->channels, m_pAFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);
				int resampledDataSize = swr_convert(m_pSwrCtx, &resampledData, aBufferSize, (const uint8_t **)m_pAFrame->extended_data, m_pAFrame->nb_samples);
				auto aBuffer = ref new Platform::Array<uint8_t>(resampledData, aBufferSize);

				DataWriter^ dataWriter = ref new DataWriter();
				dataWriter->WriteBytes(aBuffer);

				// Use decoding timestamp if presentation timestamp is not valid
				if (avPacket.pts == AV_NOPTS_VALUE && avPacket.dts != AV_NOPTS_VALUE)
				{
					avPacket.pts = avPacket.dts;
				}

				Windows::Foundation::TimeSpan pts = { LONGLONG(av_q2d(m_pFmtCtx->streams[m_nASI]->time_base) * 10000000 * avPacket.pts) };
				Windows::Foundation::TimeSpan dur = { LONGLONG(av_q2d(m_pFmtCtx->streams[m_nASI]->time_base) * 10000000 * avPacket.duration) };

				MediaStreamSample^ sample = MediaStreamSample::CreateFromBuffer(dataWriter->DetachBuffer(), pts);
				sample->Duration = dur;

				auto t = sample->Timestamp;
				PushAudioSample(sample);

				av_freep(&resampledData);
				av_frame_unref(m_pAFrame);
			}
			avPacket.size -= decodedBytes;
			avPacket.data += decodedBytes;
		}
		av_free_packet(&avPacket);
		bResult = TRUE;
	}
	
lb_return:
	return bResult;

}
BOOL CDecoder::CreateAudioQ()
{
	m_pAudioSampleQ = new std::queue<MediaStreamSample^>;
	return TRUE;
}
void CDecoder::EmptyAudioQ()
{
	if (m_pAudioSampleQ)
	{
		m_pAudioSampleQ->empty();
	}
}
void CDecoder::CleanupAudioQ()
{
	if (m_pAudioSampleQ)
	{
		m_pAudioSampleQ->empty();
		delete m_pAudioSampleQ;
		m_pAudioSampleQ = NULL;
	}
}

BOOL CDecoder::CreateVideoQ()
{
	m_pVideoPacketQ = new std::queue<AVPacket>;
	return TRUE;
}

void CDecoder::EmptyVideoQ()
{
	AcquireSRWLockExclusive(&m_LockVidoPacketQ);
	
	size_t	count = m_pVideoPacketQ->size();
	for (size_t i=0; i<count; i++)
	{
		AVPacket	avPacket = m_pVideoPacketQ->front();
		m_pVideoPacketQ->pop();
		av_free_packet(&avPacket);
	}
	
	ReleaseSRWLockExclusive(&m_LockVidoPacketQ);
}
void CDecoder::CleanupVideoQ()
{
	if (m_pVideoPacketQ)
	{
		EmptyVideoQ();
		
		delete m_pVideoPacketQ;
		m_pVideoPacketQ = NULL;
	}

}

BOOL CDecoder::IsDecodeCanceled()
{
	LONG	lValue = _InterlockedOr(&m_lCanceled,0);
	BOOL	bCanceled = (lValue == 1);

	return bCanceled;
}
void CDecoder::Cancel()
{
	_InterlockedOr(&m_lCanceled,1);
	
}
void CDecoder::ClearCanceledStatus()
{
	_InterlockedAnd(&m_lCanceled,0);

}



BOOL __stdcall CDecoder::AcquireYUVFrameWithTimeStamp(YUV_FRAME** ppOutFrame,__int64 time_stamp)
{
	YUV_FRAME*	pBuffer = m_pYUVQueue->AcquireBufferWithSec(time_stamp);
	*ppOutFrame = pBuffer;

	BOOL	bResult = (pBuffer != NULL);
	return bResult;

}
MediaStreamSource^ __stdcall CDecoder::GetAudioStreamSource()
{	
	//WaitForSingleObject(m_hDecodeReady,INFINITE);
	MediaStreamSource^	mss = m_audioProvider->GetAudioStreamSource();
	return mss;
}


void CDecoder::PushAudioSample(MediaStreamSample^ sample)
{
	size_t	count = m_pAudioSampleQ->size();
#ifdef _DEBUG
	if (count > 1000)
		__debugbreak();
#endif
	m_pAudioSampleQ->push(sample);
}


MediaStreamSample^ CDecoder::GetNextAudioSample(BOOL* pbOutDestroyed)
{
	MediaStreamSample^ sample = nullptr;
	*pbOutDestroyed = FALSE;

	if (IsDecodeCanceled())
	{
		*pbOutDestroyed = TRUE;
		goto lb_return;
	}
	size_t	count = 0;
	
	while (!(count = m_pAudioSampleQ->size()))
	{
		// 파일 끝까지 갔거나 종료된 경우 처리할것
		if (!Read())
		{
			goto lb_return;
		}
	}
	
	sample = m_pAudioSampleQ->front();
	m_pAudioSampleQ->pop();

lb_return:
	if (IsDecodeCanceled())
	{
		__debugbreak();
	}
	return sample;
}


AudioStreamDescriptor^ CDecoder::CreateAudioStreamDescriptor(AVCodecContext* pACtx,bool forceAudioDecode)
{
	AudioStreamDescriptor^ audioStreamDescriptor = nullptr;
	if (pACtx->codec_id == AV_CODEC_ID_AAC && !forceAudioDecode)
	{
		audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreateAac(pACtx->sample_rate, pACtx->channels, pACtx->bit_rate));
		//audioSampleProvider = ref new MediaSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx);
	}
	else if (pACtx->codec_id == AV_CODEC_ID_MP3 && !forceAudioDecode)
	{
		audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreateMp3(pACtx->sample_rate, pACtx->channels, pACtx->bit_rate));
		//audioSampleProvider = ref new MediaSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx);
	}
	else
	{
		// Set default 16 bits when bits per sample value is unknown (0)
		unsigned int bitsPerSample = pACtx->bits_per_coded_sample ? pACtx->bits_per_coded_sample : 16;
		audioStreamDescriptor = ref new AudioStreamDescriptor(AudioEncodingProperties::CreatePcm(pACtx->sample_rate, pACtx->channels, bitsPerSample));
		//audioSampleProvider = ref new UncompressedAudioSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx);
	}
	return audioStreamDescriptor;
}

void CDecoder::DestroyThread()
{
	
	
	// 디코딩 스레드
	if (m_hDecodeThread)
	{
		WaitForSingleObject(m_hDecodeThread,INFINITE);
		
		CloseHandle(m_hDecodeThread);
		m_hDecodeThread = NULL;
	}
	ClearCanceledStatus();
}

void CDecoder::Stop()
{
	// 디코딩 취소
	Cancel();

	//
	// 이 사이에 SampleRequest가 들어올수 있을까?
	//

	// Decode스레드 종료
	DestroyThread();
	
	// 비디오 패킷 큐 empty 및 제거
	CleanupVideoQ();

	// 오디오 패킷 큐 empty 및 제거
	CleanupAudioQ();

	// YUV프레잌 큐 제거
	CleanupYUVQueue();
	
	// ffmpeg 스트림 관련 객체들 제거
	CleanupFFMpegStream();

	if (m_pFileStream)
	{
		ULONG	ref_count = m_pFileStream->Release();
		m_pFileStream = NULL;
	}
	m_pOnDecoderEventFunc((IDecoder*)this,DECODER_EVENT_TYPE_END);
}

BOOL CDecoder::PushYUVFrame(BOOL* pbOutDestroyed,DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp)
{
	BOOL	bResult = FALSE;
	
	*pbOutDestroyed = FALSE;

	if (IsDecodeCanceled())
	{
		*pbOutDestroyed = TRUE;
		goto lb_return;
	}
	if (m_pYUVQueue)
	{
		if (m_pYUVQueue->GetWidth() != Width || m_pYUVQueue->GetHeight() != Height || m_pYUVQueue->GetStride() != Stride)
		{
			CleanupYUVQueue();
		}
	}
	if (!m_pYUVQueue)
	{
		CreateYUVQueue(Width,Height,Stride);
		m_pOnDecoderEventFunc((IDecoder*)this,DECODER_EVENT_TYPE_FIRST_FRAME_DECODED);
	}
	bResult = m_pYUVQueue->UpdateAndNext(Width,Height,pYBuffer,pUBuffer,pVBuffer,Stride,time_stamp);
lb_return:
	return bResult;
}
BOOL CDecoder::CreateYUVQueue(DWORD Width,DWORD Height,DWORD Stride)
{
	const	DWORD	MAX_FRAME_NUM = 30;
	if (m_pYUVQueue )
	{
		__debugbreak();
	}
	m_pYUVQueue = new CYUVQueue;
	m_pYUVQueue->Initialize(MAX_FRAME_NUM,Width,Height,Stride);

	return TRUE;

}
void CDecoder::EmptyYUVQueue()
{
	if (m_pYUVQueue)
	{
		m_pYUVQueue->Empty();
	}
}
void CDecoder::CleanupYUVQueue()
{
	
	if (m_pYUVQueue)
	{
		delete m_pYUVQueue;
		m_pYUVQueue = NULL;
	}
}


void CDecoder::CleanupFFMpegStream()
{
	m_audioProvider = nullptr;

	CleanupAudioQ();
	
	m_nVSI = -1;
	m_nASI = -1;
	m_pVCtx = NULL;
	m_pACtx = NULL;

	if (m_pSwrCtx)
	{
		swr_free(&m_pSwrCtx);
		m_pSwrCtx = NULL;
	}

	if (m_pVFrame)
	{
		av_free(m_pVFrame);
		m_pVFrame = NULL;
	}
	if (m_pAFrame)
	{
		av_free(m_pAFrame);
		m_pAFrame = NULL;
	}

	if (m_pFmtCtx)
	{
		for (unsigned int i=0 ; i < m_pFmtCtx->nb_streams ; i++ ) 
		{
			avcodec_close(m_pFmtCtx->streams[i]->codec); 
		}
		avformat_close_input(&m_pFmtCtx);
	}
	
	if (m_pIOCtx)
	{
		av_free(m_pIOCtx);
		m_pIOCtx = NULL;
	}
}
void CDecoder::Cleanup()
{
	Stop();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

}



CDecoder::~CDecoder()
{
	Cleanup();
}



int CDecoder::OnDecodeThread()
{
	// DecodeThrea에서 호출

	AVPacket	avPacket;
	while (1)
	{
		if (IsDecodeCanceled())
			break;

		AcquireSRWLockExclusive(&m_LockVidoPacketQ);

		size_t count = m_pVideoPacketQ->size();
		if (!count)
			goto lb_exit;

		avPacket = m_pVideoPacketQ->front();
		m_pVideoPacketQ->pop();

	lb_exit:

		ReleaseSRWLockExclusive(&m_LockVidoPacketQ);

		if (count)
		{
			BOOL	bDecodeResult = DecodePacket(avPacket);
			av_free_packet(&avPacket);

			if (!bDecodeResult)
				break;

		}
	}
	return 0;
}
BOOL CDecoder::DecodePacket(AVPacket avPacket)
{
	BOOL	bResult = FALSE;

	if (IsDecodeCanceled())
		goto lb_return;

	BOOL	bGotPicture = FALSE;
	if (avcodec_decode_video2(m_pVCtx, m_pVFrame, &bGotPicture, &avPacket) >= 0)
	{
		bResult = TRUE;
		if (bGotPicture)
		{
			// Convert packet time (here, dts) to seconds with:  
			double seconds = (avPacket.dts - m_pFmtCtx->streams[m_nVSI]->start_time) * av_q2d(m_pFmtCtx->streams[m_nVSI]->time_base);

			DWORD	stride = m_pVFrame->linesize[0];
			DWORD	stride_half = stride / 2;

#ifdef _DEBUG
			if (stride_half != m_pVFrame->linesize[1])
				__debugbreak();

			if (stride_half != m_pVFrame->linesize[2])
				__debugbreak();

#endif
			DWORD		width = m_pVCtx->width;
			DWORD		width_half = width / 2;

			DWORD		height = m_pVCtx->height;

			// Use decoding timestamp if presentation timestamp is not valid
			if (avPacket.pts == AV_NOPTS_VALUE && avPacket.dts != AV_NOPTS_VALUE)
			{
				avPacket.pts = avPacket.dts;
			}

			Windows::Foundation::TimeSpan pts = { LONGLONG(av_q2d(m_pFmtCtx->streams[m_nVSI]->time_base) * 10000000 * avPacket.pts) };
			Windows::Foundation::TimeSpan dur = { LONGLONG(av_q2d(m_pFmtCtx->streams[m_nVSI]->time_base) * 10000000 * avPacket.duration) };

			__int64 time_stamp = pts.Duration;
			
			BOOL	bDestroyed = FALSE;
			while (FALSE == PushYUVFrame(&bDestroyed, width, height, m_pVFrame->data[0], m_pVFrame->data[1], m_pVFrame->data[2], stride, time_stamp))
			{
				if (bDestroyed)
				{
					bResult = FALSE;
					break;
				}
				else
				{
					Sleep(1);
				}
			}
		}
	}
lb_return:
	return bResult;
}
