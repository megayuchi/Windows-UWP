#include "pch.h"
#include "../Common/decoder_typedef.h"
#include "Util.h"
#include "Decoder.h"

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}
#include "CustomStreamFunc.h"
#include "../Common_UTIL/debug_new.h"

int ReadStreamFunc(void* ptr, uint8_t* buf, int buf_size)
{
	IStream*	pFileStream = (IStream*)ptr;

	
	ULONG bytesRead = 0;
	HRESULT hr = pFileStream->Read(buf, buf_size, &bytesRead);

	if (FAILED(hr))
	{
		return -1;
	}

	// If we succeed but don't have any bytes, assume end of file
	if (bytesRead == 0)
	{
		return AVERROR_EOF;  // Let FFmpeg know that we have reached eof
	}

	return (int)bytesRead;
}
// whence: SEEK_SET, SEEK_CUR, SEEK_END (like fseek) and AVSEEK_SIZE

int64_t SeekStreamFunc(void* ptr, int64_t pos, int whence)
{
	IStream*	pFileStream = (IStream*)ptr;

	LARGE_INTEGER in;
	in.QuadPart = pos;
	ULARGE_INTEGER out = { 0 };

	if (FAILED(pFileStream->Seek(in, whence, &out)))
	{
		return -1;
	}

	return out.QuadPart; // Return the new position:
}
 
