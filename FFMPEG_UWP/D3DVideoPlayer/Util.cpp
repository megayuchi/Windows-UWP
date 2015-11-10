#include "pch.h"
#include "Util.h"

#include "../Common_UTIL/debug_new.h"

_CrtMemState	g_MemState = {0};

void BeginHeapCheck()
{
#ifdef _DEBUG
	int	flag = _CRTDBG_ALLOC_MEM_DF |_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif
	
	_CrtMemCheckpoint(&g_MemState);
}

void EndHeapCheck()
{
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
	//int	leak = _CrtDumpMemoryLeaks();
	//_CrtMemCheckpoint(&g_MemState);
	_CrtMemDumpAllObjectsSince(&g_MemState);
#endif
}


DWORD GetSizeText(WCHAR* wchOutTxt,DWORD dwMaxCount,UINT64 ui64Size)
{
	DWORD	dwLen = 0;
	if (ui64Size >= 1024 * 1024 * 1024)
	{
		float SizeInGB = ((float)ui64Size / (float)(1024 * 1024 * 1024));
		dwLen = swprintf_s(wchOutTxt,dwMaxCount,L"%.2fGB",SizeInGB);
		goto lb_return;
	}
	if (ui64Size >= 1024 * 1024)
	{
		float	SizeInMB = ((float)ui64Size / (float)(1024 * 1024));
		dwLen = swprintf_s(wchOutTxt,dwMaxCount,L"%.2fMB",SizeInMB);
		goto lb_return;
	}
	if (ui64Size >= 1024)
	{
		float	SizeInKB = (float)ui64Size / 1024.0f;
		dwLen = swprintf_s(wchOutTxt,dwMaxCount,L"%.2fKB",SizeInKB);
		goto lb_return;
	}
	dwLen = swprintf_s(wchOutTxt,dwMaxCount,L"%lluB",ui64Size);
lb_return:
	return dwLen;
}
DWORD CreateDllFileName(WCHAR* wchOutFileName,DWORD dwMaxBufferCount,WCHAR* wchName,CPU_ARCH arch,BUILD_CONFIG config)
{	
	WCHAR*	wchOS = NULL;
	WCHAR*	wchArch = NULL;
	WCHAR*	wchConfig = NULL;

	switch(arch)
	{
	case CPU_ARCH_X86:
		wchArch = L"_x86";
		break;
	case CPU_ARCH_X64:
		wchArch = L"_x64";
		break;
	case CPU_ARCH_ARM:
		wchArch = L"_arm";
		break;
	}

	switch(config)
	{
	case BUILD_CONFIG_DEBUG:
		wchConfig = L"_debug";
		break;
	case BUILD_CONFIG_RELEASE:
		wchConfig = L"_release";
		break;

	}
	if (wchName)
	{
		wcscpy_s(wchOutFileName,dwMaxBufferCount,wchName);
	}
	if (wchOS)
	{
		wcscat_s(wchOutFileName,dwMaxBufferCount,wchOS);
	}
	if (wchArch)
	{
		wcscat_s(wchOutFileName,dwMaxBufferCount,wchArch);
	}
	if (wchConfig)
	{
		wcscat_s(wchOutFileName,dwMaxBufferCount,wchConfig);
	}
	wcscat_s(wchOutFileName,dwMaxBufferCount,L".dll");
	DWORD	dwLen = (DWORD)wcslen(wchOutFileName);

	return dwLen;
}


void __stdcall CharToSmallASCIILen(char* szDest,char* szStr)
{
	DWORD	dwLen = (DWORD)strlen(szStr);
	
	CharToSmallASCII(szDest,szStr,dwLen);

}
void __stdcall CharToSmallASCII(char* szDest,char* szStr,DWORD dwLen)
{
	for (DWORD i=0; i<dwLen; i++)
	{
		szDest[i] = szStr[i];

		if (szDest[i] < 'A')
			continue;
		
		if (szDest[i] > 'Z')
			continue;

		szDest[i] += 32;
		
	}
}
void __stdcall WCharToSmallWChar(WCHAR* wchDest,WCHAR* wchSrc,DWORD dwLen)
{
	int			Offset = L'a' - L'A';
	for (DWORD i=0; i<dwLen; i++)
	{
		if (wchSrc[i] > L'A' && wchDest[i] < L'Z')
			wchDest[i] = wchSrc[i] + Offset;
	}	
}

DWORD __stdcall	RemoveExt(char* szResultName,char* szFileName)
{
	DWORD	dwSrcLen = (DWORD)strlen(szFileName);
	DWORD	dwCount = dwSrcLen;


	while (dwCount)
	{
		dwCount--;
		if (szFileName[dwCount] == '.')
		{
			memcpy(szResultName,szFileName,dwCount);
			break;
		}
		
		
	}
	szResultName[dwCount] = 0;
	return dwCount;
}

DWORD __stdcall	GetExt(char* szResultName,DWORD dwMaxLen,char* szFileName)
{
	DWORD	dwLen = 0;
	
	DWORD len_src = (DWORD)strnlen(szFileName,_MAX_PATH);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (!dwMaxLen)
			goto lb_return;

		dwMaxLen--;

		if (szFileName[i-1] == '.')
		{
			dwLen = len_src-i+1;
			memcpy(szResultName,szFileName+i-1,dwLen);
			goto lb_return;
		}
		
	}
	
	

lb_return:
	szResultName[dwLen] = 0;
	return dwLen;
}
DWORD __stdcall GetNamePath(char* szResultPath,char* szFileName)
{
	DWORD	dwLen;
	
	DWORD len_src = (DWORD)strlen(szFileName);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (szFileName[i-1] == '\\' || szFileName[i-1] == '/')
		{
			memcpy(szResultPath,szFileName,i);
			dwLen = i;
			goto lb_return;
		}
		
	}
	dwLen = 0;
	

lb_return:
	szResultPath[dwLen] = 0;
	return dwLen;

}

DWORD __stdcall GetNameRemovePath(char* dest,char* src)
{
	
	if (!src || !dest)
		return 0;

	DWORD	dwLen;
	

	DWORD len_src = (DWORD)strlen(src);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (src[i-1] == '\\' || src[i-1] == '/')
		{
			memcpy(dest,&src[i],len_src-i);
			dwLen = len_src-i;
			goto lb_return;
		}
		
	}
	memcpy(dest,src,len_src);
	dwLen = len_src;

lb_return:
	dest[dwLen] = 0;
	return dwLen;

}

DWORD __stdcall GetSuffixStringFromExt(char* szDest,char* szFileName,DWORD dwLenFromRight)
{
	char	szTempPath[_MAX_PATH];
	DWORD	dwResult = 0;
    DWORD dwLen = RemoveExt(szTempPath,szFileName);
	DWORD i,dwGetLen;

	if (dwLen < dwLenFromRight)
		goto lb_return;

	dwGetLen = dwLen - dwLenFromRight;

	for (i=0; i<dwLenFromRight; i++)
	{
		szDest[i] = szTempPath[dwGetLen+i];
	}
	szDest[i] = 0;
	dwResult = dwLenFromRight;

lb_return:
	return dwResult;
}
DWORD __stdcall GetRightString(char* szDest,char* szSrc,DWORD dwLenFromRight)
{
	DWORD	i,dwResult = 0;
	DWORD dwLen = (DWORD)strlen(szSrc);
	char*	pRight = szSrc;

	if (dwLen <= dwLenFromRight)
		goto lb_return;

	pRight += (dwLen - dwLenFromRight);
	

	for (i=0; i<dwLenFromRight; i++)
	{
		szDest[i] = pRight[i];
		
	}
	szDest[i] = 0;
	dwResult = dwLenFromRight;

lb_return:
	return dwResult;
}
DWORD __stdcall	RemoveExtW(WCHAR* wchResultName,WCHAR* wchFileName)
{
	DWORD	dwSrcLen = (DWORD)wcslen(wchFileName);
	DWORD	dwCount = dwSrcLen;


	while (dwCount)
	{
		dwCount--;
		if (wchFileName[dwCount] == '.')
		{
			memcpy(wchResultName,wchFileName,sizeof(WCHAR)*dwCount);
			break;
		}
		
		
	}
	wchResultName[dwCount] = 0;
	return dwCount;
}
DWORD __stdcall	GetExtW(WCHAR* wchResultName,DWORD dwMaxLen,WCHAR* wchFileName)
{
	DWORD	dwLen = 0;
	
	DWORD len_src = (DWORD)wcsnlen(wchFileName,_MAX_PATH);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (!dwMaxLen)
			goto lb_return;

		dwMaxLen--;

		if (wchFileName[i-1] == '.')
		{
			dwLen = len_src-i+1;
			memcpy(wchResultName,wchFileName+i-1,sizeof(WCHAR)*dwLen);
			goto lb_return;
		}
		
	}
	
	

lb_return:
	wchResultName[dwLen] = 0;
	return dwLen;
}

DWORD __stdcall GetNamePathW(WCHAR* wchResultPath,WCHAR* wchFileName)
{
	DWORD	dwLen;
	
	DWORD len_src = (DWORD)wcslen(wchFileName);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (wchFileName[i-1] == '\\' || wchFileName[i-1] == '/')
		{
			memcpy(wchResultPath,wchFileName,sizeof(WCHAR)*i);
			dwLen = i;
			goto lb_return;
		}
		
	}
	dwLen = 0;
	

lb_return:
	wchResultPath[dwLen] = 0;
	return dwLen;

}
DWORD __stdcall GetNameRemovePathW(WCHAR* dest,const WCHAR* src)
{
	
	if (!src || !dest)
		return 0;

	DWORD	dwLen;
	

	DWORD len_src = (DWORD)wcslen(src);
	DWORD count = 0;
	DWORD i=0;

	for (i=len_src; i>0; i--)
	{
		if (src[i-1] == '\\' || src[i-1] == '/')
		{
			memcpy(dest,&src[i],sizeof(WCHAR)*(len_src-i));
			dwLen = len_src-i;
			goto lb_return;
		}
		
	}
	memcpy(dest,src,sizeof(WCHAR)*len_src);
	dwLen = len_src;

lb_return:
	dest[dwLen] = 0;
	return dwLen;

}


DWORD __stdcall GetSuffixStringFromExtW(WCHAR* wchDest,WCHAR* wchFileName,DWORD dwLenFromRight)
{
	WCHAR	wchTempPath[_MAX_PATH];
	DWORD	dwResult = 0;
    DWORD dwLen = RemoveExtW(wchTempPath,wchFileName);
	DWORD i,dwGetLen;

	if (dwLen < dwLenFromRight)
		goto lb_return;

	dwGetLen = dwLen - dwLenFromRight;

	for (i=0; i<dwLenFromRight; i++)
	{
		wchDest[i] = wchTempPath[dwGetLen+i];
	}
	wchDest[i] = 0;
	dwResult = dwLenFromRight;

lb_return:
	return dwResult;
}
DWORD __stdcall GetRightStringW(WCHAR* wchDest,WCHAR* wchSrc,DWORD dwLenFromRight)
{
	DWORD	i,dwResult = 0;
	DWORD dwLen = (DWORD)wcslen(wchSrc);
	WCHAR*	pRight = wchSrc;

	if (dwLen <= dwLenFromRight)
		goto lb_return;

	pRight += (dwLen - dwLenFromRight);
	

	for (i=0; i<dwLenFromRight; i++)
	{
		wchDest[i] = pRight[i];
		
	}
	wchDest[i] = 0;
	dwResult = dwLenFromRight;

lb_return:
	return dwResult;
}

DWORD __stdcall RemoveWhiteSpace_ASCII(char* szDest,int iMaxCount,char* szSrc,int iLen)
{
	int		iStartOffset = 0;
	int		iSpaceCount = 0;

	iMaxCount--;

	for (int i=0; i<iLen; i++)
	{
		int	c = (int)(DWORD)szSrc[i];

		if (isspace(c))
		{
			iSpaceCount++;
		}
		else
		{
			iStartOffset = i;
			break;
		}
	}
	iLen -= iSpaceCount;

	if (iLen > iMaxCount)
		iLen = iMaxCount;
	
	memcpy(szDest,szSrc+iStartOffset,iLen);
	szDest[iLen] = 0;


	for (int i=iLen-1; i>= 0; i--)
	{
		int	c = (int)(DWORD)szDest[i];

		if (isspace(c))
		{
			iLen--;
		}
		else
		{
			break;
		}
	}
	szDest[iLen] = 0;
	

	return iLen;
}

DWORD __stdcall RemoveWhiteSpace_WIDE(WCHAR* wchDest,int iMaxCount,WCHAR* wchSrc,int iLen)
{
	int		iStartOffset = 0;
	int		iSpaceCount = 0;

	iMaxCount--;

	for (int i=0; i<iLen; i++)
	{
		int	c = (int)(DWORD)wchSrc[i];

		if (iswspace(c))
		{
			iSpaceCount++;
		}
		else
		{
			iStartOffset = i;
			break;
		}
	}
	iLen -= iSpaceCount;

	if (iLen > iMaxCount)
		iLen = iMaxCount;
	
	memcpy(wchDest,wchSrc+iStartOffset,iLen*sizeof(WCHAR));
	wchDest[iLen] = 0;


	for (int i=iLen-1; i>= 0; i--)
	{
		int	c = (int)(DWORD)wchDest[i];

		if (iswspace(c))
		{
			iLen--;
		}
		else
		{
			break;
		}
	}
	wchDest[iLen] = 0;
	

	return iLen;
}

BOOL __stdcall RemoveCRLF_ASCII(char* pStr,DWORD dwLen)
{	
	BOOL	bResult = FALSE;

	for (DWORD i=0; i<dwLen; i++)
	{
		if (pStr[i] == 0x0d)
		{
			pStr[i] = 0x20;
			bResult = TRUE;
		}
		else if (pStr[i] == 0x0a)
		{
			pStr[i] = 0x20;
			bResult = TRUE;
		}
		else if (pStr[i] == 0x09)
		{
			pStr[i] = 0x20;
			bResult = TRUE;
		}
	}
	return bResult;

}

BOOL __stdcall RemoveCRLF_Wide(WCHAR* pStr,DWORD dwLen)
{	
	BOOL	bResult = FALSE;

	for (DWORD i=0; i<dwLen; i++)
	{
		if (pStr[i] == 0x000d)
		{
			pStr[i] = 0x0020;
			bResult = TRUE;
		}
		else if (pStr[i] == 0x000a)
		{
			pStr[i] = 0x0020;
			bResult = TRUE;
		}
		else if (pStr[i] == 0x0009)
		{
			pStr[i] = 0x0020;
			bResult = TRUE;
		}
	}
	return bResult;

}
DWORD __stdcall GetRemoveWhiteSpace_ASCII(char* pOutBuffer,DWORD dwMaxBufferSize,char** ppOutNext,char* pSrc,DWORD dwLen)
{
	DWORD	dwResult = 0;


	char*		pLast = NULL;
	if (dwMaxBufferSize < dwLen)
		goto lb_return;

	
	for (DWORD i=0; i<dwLen; i++)
	{
		int		c = (int)(DWORD)pSrc[i];
		if (!isspace(c))
		{
			pLast = pSrc+i;
			pOutBuffer[dwResult] = c;
			dwResult++;
		}
	}
	*ppOutNext = pLast+1;
	pOutBuffer[dwResult] = 0;


lb_return:
	return dwResult;
}

ULONG GetRefCount(IUnknown* pObj)
{
	pObj->AddRef();
	ULONG	ref = pObj->Release();

	return ref;
}
/*
DWORD WINAPI GetFileSize(__in HANDLE hFile, __out_opt LPDWORD lpFileSizeHigh)
{
	BOOL	dwFileSizeLow = 0;

	struct FILE_STREAM_INFO_EXT : FILE_STREAM_INFO
	{
		WCHAR ExtSize[128];
	};
	FILE_STREAM_INFO_EXT	info;
	memset(&info, 0, sizeof(info));

	if (!GetFileInformationByHandleEx(hFile, FileStreamInfo,&info,sizeof(info)))
		goto lb_return;

	if (lpFileSizeHigh)
	{
		*lpFileSizeHigh = info.StreamSize.HighPart;
	}
	dwFileSizeLow = info.StreamSize.LowPart;
	
lb_return:
	return dwFileSizeLow;
}
*/
/*
BOOL WINAPI GetFileTime(__in HANDLE hFile, __out_opt LPFILETIME lpCreationTime, __out_opt LPFILETIME lpLastAccessTime, __out_opt LPFILETIME lpLastWriteTime)
{
	BOOL	bResult = FALSE;
	
	FILE_BASIC_INFO	info = {0};
	
	bResult = GetFileInformationByHandleEx(hFile, FileBasicInfo, &info, sizeof(info));
	
	if (lpCreationTime)
	{
		*lpCreationTime = *(FILETIME*)&info.CreationTime;
	}
	if (lpLastAccessTime)
	{
		*lpLastAccessTime = *(FILETIME*)&info.LastAccessTime;
	}
	if (lpLastWriteTime)
	{
		*lpLastWriteTime = *(FILETIME*)&info.LastWriteTime;
	}

	
lb_return:

	return bResult;
}
*/
