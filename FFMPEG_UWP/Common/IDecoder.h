#pragma once

using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;

#include "decoder_typedef.h"

interface IDecoder : public IUnknown
{
	virtual BOOL	__stdcall	Initialize(OnDecoderCallback pFunc) = 0;
	virtual	BOOL	__stdcall	Decode(IStream* pFileStream) = 0;
	virtual void	__stdcall	Stop() = 0;
	virtual BOOL	__stdcall	AcquireYUVFrameWithTimeStamp(YUV_FRAME** ppOutFrame,__int64 time_stamp) = 0;
	virtual MediaStreamSource^	__stdcall	GetAudioStreamSource() = 0;
};