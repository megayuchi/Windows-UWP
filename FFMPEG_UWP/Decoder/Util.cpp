#include "pch.h"

#include <stdio.h>
#include "../Common/decoder_typedef.h"
#include "Util.h"
#include "Decoder.h"

#include "../Common_UTIL/debug_new.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;



size_t GetFileSizeWithFP(FILE* fp)
{
	long	old_pos = ftell(fp);
	fseek(fp,0,SEEK_END);
	size_t	size = ftell(fp);
	
	fseek(fp,old_pos,SEEK_SET);

	return size;
}

BOOL GetRefCount(IUnknown* pObj)
{
	pObj->AddRef();
	ULONG	ref_count = pObj->Release();
	return ref_count;
}