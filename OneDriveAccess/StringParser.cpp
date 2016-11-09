#include "pch.h"
#include <Windows.h>
#include "StringParser.h"

const WCHAR* wchAccessTokenKey = L"access_token";

DWORD GetToken(WCHAR* wchOutToken,DWORD dwMaxBufferCount,const WCHAR* frag)
{
	DWORD	dwTokenLen = 0;

	WCHAR*	pBufStart = nullptr;
	WCHAR* pBuf = nullptr;

	size_t len = wcslen(frag);

	if (len < 16)
		goto lb_return;
		
	pBuf = new WCHAR[len + 1];
	memcpy(pBuf, frag, len * sizeof(WCHAR));
	pBuf[len] = 0;
	

	for (DWORD i = 0; i < len; i++)
	{
		if (pBuf[i] == '#')
		{
			pBufStart = pBuf + i+1;
			break;
		}
	}
	if (!pBufStart)
		goto lb_return;

	WCHAR*	token = nullptr;
	WCHAR*	next_token = nullptr;
	WCHAR	seps[] = L"&";
	token = wcstok_s(pBufStart, seps, &next_token);

	while (token)
	{
		dwTokenLen = GetKeyValue(wchOutToken, dwMaxBufferCount, token);
		if (dwTokenLen)
		{
			break;
		}
		else
		{
			
		}
		token = wcstok_s(nullptr, seps, &next_token);
	}
lb_return:
	if (pBuf)
	{
		delete[] pBuf;
		pBuf = nullptr;
	}
	return dwTokenLen;
}

DWORD GetKeyValue(WCHAR* wchOutToken,DWORD dwMaxBufferCount,const WCHAR* parts)
{
	
	DWORD	dwTokenLen = 0;
	WCHAR*	pBuf = nullptr;

	size_t len = wcslen(parts);
	if (len < 2)
		goto lb_return;

	pBuf = new WCHAR[len + 1];
	memcpy(pBuf, parts, sizeof(WCHAR)*len);
	pBuf[len] = 0;

	WCHAR*	pBufStart = pBuf;

	WCHAR*	token = nullptr;
	WCHAR*	next_token = nullptr;
	WCHAR	seps[] = L"=";
	token = wcstok_s(pBufStart, seps, &next_token);

	while (token)
	{
		
		if (!wcscmp(token, wchAccessTokenKey))
		{
			if (next_token)
			{
				dwTokenLen = (DWORD)wcslen(next_token);
				if (dwMaxBufferCount <= dwTokenLen)
					__debugbreak();

				memcpy(wchOutToken, next_token, sizeof(WCHAR)*dwTokenLen);
				wchOutToken[dwTokenLen] = 0;
				break;
			}
		}
		token = wcstok_s(nullptr, seps, &next_token);
	}
	
lb_return:
	if (pBuf)
	{
		delete[] pBuf;
		pBuf = nullptr;
	}
	return dwTokenLen;
}