#include "pch.h"
#include "../Common/decoder_typedef.h"
#include "DecoderThread.h"
#include "Decoder.h"

#include <process.h>
#include "../Common_UTIL/debug_new.h"

UINT WINAPI DecodeThread(LPVOID lpVoid)
{
	CDecoder*	pDecoder = (CDecoder*)lpVoid;
	
	pDecoder->OnDecodeThread();
	
	_endthreadex(0);
	return 0;

}
