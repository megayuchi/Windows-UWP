#pragma once

#include <Windows.h>

struct YUV_FRAME
{
	__int64 time_stamp;
	DWORD	Width;
	DWORD	Height;
	DWORD	Stride;

	BYTE*	pYBuffer;
	BYTE*	pUBuffer;
	BYTE*	pVBuffer;

	BYTE	pBuffer[1];
};

enum DECODER_EVENT_TYPE
{
	DECODER_EVENT_TYPE_START,
	DECODER_EVENT_TYPE_END,
	DECODER_EVENT_TYPE_FIRST_FRAME_DECODED



};
typedef void (__stdcall *OnDecoderCallback)(void* pData,DECODER_EVENT_TYPE type);