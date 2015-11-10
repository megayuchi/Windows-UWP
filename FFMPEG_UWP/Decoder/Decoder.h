#pragma once

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
}


#include "../Common/IDecoder.h"
#include "AudioProvider.h"
#include<queue>

using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace MediaSourceProvider;

enum DECODER_STATUS
{
	DECODER_STATUS_NOT_WORKING,
	DECODER_STATUS_ANALYZING_FORMAT,
	DECODER_STATUS_BEGIN_DECODING,
	DECODER_STATUS_DECODING_GOT_PICTURE,

};

class CYUVQueue;

class CDecoder : public IDecoder
{
	DWORD			m_dwRefCount;
	HANDLE			m_hDecodeThread;
	CYUVQueue*		m_pYUVQueue = nullptr;
	IStream*		m_pFileStream = nullptr;
	AudioProvider^	m_audioProvider = nullptr;
	SRWLOCK			m_LockVidoPacketQ;

	volatile	LONG	m_lCanceled;
	
	std::queue<MediaStreamSample^>* m_pAudioSampleQ;
	std::queue<AVPacket>* m_pVideoPacketQ;

	AVFormatContext* m_pFmtCtx = NULL; 
	AVIOContext*	m_pIOCtx = NULL;
	AVFrame*		m_pVFrame = NULL;
	AVFrame*		m_pAFrame = NULL;
	SwrContext*		m_pSwrCtx = NULL;
	
	AVCodecContext* m_pVCtx = NULL;
	AVCodecContext* m_pACtx = NULL;
	int				m_nVSI = -1;
	int				m_nASI = -1;

	OnDecoderCallback		m_pOnDecoderEventFunc = nullptr;
	BOOL	InitFFMpegStream();
	void	CleanupFFMpegStream();

	

	BOOL	Read();
	BOOL	PushYUVFrame(BOOL* pbOutDestroyed,DWORD Width,DWORD Height,BYTE* pYBuffer,BYTE* pUBuffer,BYTE* pVBuffer,DWORD Stride,__int64 time_stamp);

	BOOL	CreateYUVQueue(DWORD Width, DWORD Height, DWORD Stride);
	void	CleanupYUVQueue();
	
	
	
	void	DestroyThread();
	void	ClearCanceledStatus();
	
	AudioStreamDescriptor^	CreateAudioStreamDescriptor(AVCodecContext* pACtx,bool forceAudioDecode);
	
	BOOL	CreateAudioQ();
	void	CleanupAudioQ();
	
	BOOL	CreateVideoQ();
	void	CleanupVideoQ();

	BOOL	DecodePacket(AVPacket avPacket);
	void	Cleanup();
	
	
public:
	void	Cancel();
	BOOL	IsDecodeCanceled();
		
	void	PushAudioSample(MediaStreamSample^ sample);
	void	EmptyAudioQ();
	void	EmptyVideoQ();
	void	EmptyYUVQueue();

	MediaStreamSample^ CDecoder::GetNextAudioSample(BOOL* pbOutDestroyed);

	
	
	
	
	int		OnDecodeThread();	// DecodeThrea에서 호출

	

	// Derrived from IDecoder
	STDMETHODIMP			QueryInterface(REFIID refiid, void** ppv);
	STDMETHODIMP_(ULONG)	AddRef(void);
	STDMETHODIMP_(ULONG)	Release(void);

	BOOL	__stdcall	Initialize(OnDecoderCallback pFunc);
	BOOL	__stdcall	Decode(IStream* pFileStream);
	void	__stdcall	Stop();
	
	
	BOOL	__stdcall	AcquireYUVFrameWithTimeStamp(YUV_FRAME** ppOutFrame,__int64 time_stamp);
	
	MediaStreamSource^	__stdcall	GetAudioStreamSource();
	

	CDecoder();
	~CDecoder();
};

