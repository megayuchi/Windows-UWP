#pragma once


DWORD GetToken(WCHAR* wchOutToken, DWORD dwMaxBufferCount, const WCHAR* frag);
DWORD GetKeyValue(WCHAR* wchOutToken, DWORD dwMaxBufferCount, const WCHAR* parts);