#pragma once

enum CPU_ARCH
{
	CPU_ARCH_X86,
	CPU_ARCH_X64,
	CPU_ARCH_ARM
};

enum BUILD_CONFIG
{
	BUILD_CONFIG_DEBUG,
	BUILD_CONFIG_RELEASE
};

DWORD CreateDllFileName(WCHAR* wchOutFileName,DWORD dwMaxBufferCount,WCHAR* wchName,CPU_ARCH arch,BUILD_CONFIG config);
ULONG GetRefCount(IUnknown* pObj);
//DWORD WINAPI GetFileSize(__in HANDLE hFile, __out_opt LPDWORD lpFileSizeHigh);
//BOOL WINAPI GetFileTime(__in HANDLE hFile, __out_opt LPFILETIME lpCreationTime, __out_opt LPFILETIME lpLastAccessTime, __out_opt LPFILETIME lpLastWriteTime);
DWORD GetSizeText(WCHAR* wchOutTxt,DWORD dwMaxCount,UINT64 ui64Size);

void BeginHeapCheck();
void EndHeapCheck();

void __stdcall CharToSmallASCIILen(char* szDest,char* szStr);
void __stdcall CharToSmallASCII(char* szDest,char* szStr,DWORD dwLen);
void __stdcall WCharToSmallWChar(WCHAR* wchDest,WCHAR* wchSrc,DWORD dwLen);
DWORD __stdcall	RemoveExt(char* szResultName,char* szFileName);
DWORD __stdcall	GetExt(char* szResultName,DWORD dwMaxLen,char* szFileName);
DWORD __stdcall GetNamePath(char* szResultPath,char* szFileName);
DWORD __stdcall GetNameRemovePath(char* dest,char* src);
DWORD __stdcall GetSuffixStringFromExt(char* szDest,char* szFileName,DWORD dwLenFromRight);
DWORD __stdcall GetRightString(char* szDest,char* szSrc,DWORD dwLenFromRight);
DWORD __stdcall	RemoveExtW(WCHAR* wchResultName,WCHAR* wchFileName);
DWORD __stdcall	GetExtW(WCHAR* wchResultName,DWORD dwMaxLen,WCHAR* wchFileName);
DWORD __stdcall GetNamePathW(WCHAR* wchResultPath,WCHAR* wchFileName);
DWORD __stdcall GetNameRemovePathW(WCHAR* dest,const WCHAR* src);
DWORD __stdcall GetSuffixStringFromExtW(WCHAR* wchDest,WCHAR* wchFileName,DWORD dwLenFromRight);
DWORD __stdcall GetRightStringW(WCHAR* wchDest,WCHAR* wchSrc,DWORD dwLenFromRight);
DWORD __stdcall RemoveWhiteSpace_ASCII(char* szDest,int iMaxCount,char* szSrc,int iLen);
DWORD __stdcall RemoveWhiteSpace_WIDE(WCHAR* wchDest,int iMaxCount,WCHAR* wchSrc,int iLen);
BOOL __stdcall RemoveCRLF_ASCII(char* pStr,DWORD dwLen);
BOOL __stdcall RemoveCRLF_Wide(WCHAR* pStr,DWORD dwLen);
DWORD __stdcall GetRemoveWhiteSpace_ASCII(char* pOutBuffer,DWORD dwMaxBufferSize,char** ppOutNext,char* pSrc,DWORD dwLen);


