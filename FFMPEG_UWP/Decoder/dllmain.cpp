#include "pch.h"
#include "Decoder.h"

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


STDAPI DllCreateInstance(void** ppv)
{
	HRESULT hr = S_OK;
	IDecoder*	pDecoder = new CDecoder;
	if (!pDecoder)
	{
		hr = E_OUTOFMEMORY;
		goto lb_return;
	}
	hr = S_OK;
	*ppv = pDecoder;
lb_return:
	return hr;
}