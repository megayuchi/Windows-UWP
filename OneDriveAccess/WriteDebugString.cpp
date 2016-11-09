#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "WriteDebugString.h"

void WriteDebugStringW(const WCHAR* wchFormat,...)
{
#ifdef _DEBUG
	va_list argptr;
	WCHAR cBuf[2048];

	va_start( argptr, wchFormat );
	vswprintf_s( cBuf,2048, wchFormat, argptr );
	va_end( argptr );
	
	OutputDebugStringW(cBuf);
#endif
}

void WriteDebugStringA(const char* szFormat,...)
{
#ifdef _DEBUG
	va_list argptr;
	char cBuf[2048];

	va_start( argptr, szFormat );
	vsprintf_s( cBuf,2048, szFormat, argptr );
	va_end( argptr );

	OutputDebugStringA(cBuf);
#endif
	
}
