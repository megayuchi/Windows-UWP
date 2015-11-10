#include "pch.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "D3DHelper.h"

#include "../Common_UTIL/debug_new.h"

D3D_FEATURE_LEVEL	g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
BOOL CreateShaderCodeFromFile(BYTE** ppOutCodeBuffer,DWORD* pdwOutCodeSize,SYSTEMTIME* pOutLastWriteTime,char* szFileName)
{
	BOOL	bResult = FALSE;

	DWORD	dwOpenFlag = OPEN_EXISTING;
	DWORD	dwAccessMode = GENERIC_READ;
	DWORD	dwShare	=	FILE_SHARE_READ;

	WCHAR	wchTxt[128] = {0};

	WCHAR	wchFileName[_MAX_PATH];
	swprintf_s(wchFileName, L"%S", szFileName);

	CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {0};
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = NULL;
    extendedParams.hTemplateFile = NULL;

	HANDLE	hFile = CreateFile2(wchFileName,dwAccessMode,dwShare,dwOpenFlag,&extendedParams);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		swprintf_s(wchTxt,L"Shader File Not Found : %S\n",szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_return;
	}
	LARGE_INTEGER	FileSize;
	GetFileSizeEx(hFile,&FileSize);

	//DWORD	dwFileSize = GetFileSize(hFile,NULL);
	if (FileSize.QuadPart > 1024*1024)
	{
		swprintf_s(wchTxt,L"Invalid Shader File : %S\n",szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_close_return;
	}
	DWORD	dwFileSize = FileSize.LowPart;

	DWORD	dwCodeSize = dwFileSize + 1;

	BYTE*	pCodeBuffer = new BYTE[dwCodeSize];
	memset(pCodeBuffer,0,dwCodeSize);

	DWORD	dwReadBytes = 0;
	if (!ReadFile(hFile,pCodeBuffer,dwFileSize,&dwReadBytes,NULL))
	{
		swprintf_s(wchTxt,L"Failed to Read File : %S\n",szFileName);
		OutputDebugStringW(wchTxt);
		goto lb_close_return;
	}
	FILETIME	createTime,lastAccessTime,lastWriteTime;

	GetFileTime(hFile,&createTime,&lastAccessTime,&lastWriteTime);

	SYSTEMTIME	sysLastWriteTime;
	FileTimeToSystemTime(&lastWriteTime,&sysLastWriteTime);


	*ppOutCodeBuffer = pCodeBuffer;
	*pdwOutCodeSize = dwCodeSize;
	*pOutLastWriteTime = sysLastWriteTime;
	bResult = TRUE;

lb_close_return:
	CloseHandle(hFile);
	
lb_return:
	return bResult;

}
void DeleteShaderCode(BYTE* pCodeBuffer)
{
	delete [] pCodeBuffer;
}
HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut,DWORD ShaderParams,SYSTEMTIME* pOutLastWriteTime)
{
    HRESULT hr = E_FAIL;


	SYSTEMTIME	lastWriteTime;
	BYTE*		pCodeBuffer = NULL;
	DWORD		dwCodeSize = 0;
	if (!CreateShaderCodeFromFile(&pCodeBuffer,&dwCodeSize,&lastWriteTime,szFileName))
	{
		return E_FAIL;
	}

	DWORD	dwOptimizeFlag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
	DWORD	dwDebugFlag = 0;
   
#if defined( DEBUG ) || defined( _DEBUG )
	dwOptimizeFlag = D3DCOMPILE_SKIP_OPTIMIZATION;
	
    //dwDebugFlag = D3DCOMPILE_DEBUG;
#endif
	DWORD	dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	dwShaderFlags |= (dwDebugFlag | dwOptimizeFlag);
		
	D3D_SHADER_MACRO pDefine[] = {0};
	
	ID3DBlob* pErrorBlob = NULL;

	hr = D3DCompile2(pCodeBuffer,(size_t)dwCodeSize,szFileName,pDefine,D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,dwShaderFlags, 0, 0,NULL, 0,ppBlobOut, &pErrorBlob);

	DeleteShaderCode(pCodeBuffer);

    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob )
			pErrorBlob->Release();

		__debugbreak();
        return hr;
    }
    if( pErrorBlob )
		pErrorBlob->Release();
	
	*pOutLastWriteTime = lastWriteTime;
	hr = S_OK;

    return hr;
}

void SetDefaultReasterizeValue(D3D11_RASTERIZER_DESC* pOutDesc)
{
	pOutDesc->FillMode = D3D11_FILL_SOLID;
	pOutDesc->CullMode = D3D11_CULL_BACK;
	pOutDesc->FrontCounterClockwise = FALSE;
	pOutDesc->DepthBias = 0;
	pOutDesc->DepthBiasClamp = 0.0f;
	pOutDesc->SlopeScaledDepthBias = 0.0f;
	pOutDesc->DepthClipEnable = TRUE;
	pOutDesc->ScissorEnable = FALSE;
	pOutDesc->MultisampleEnable = FALSE;
	pOutDesc->AntialiasedLineEnable = FALSE;
	
}

void SetDefaultBlendValue(D3D11_BLEND_DESC* pOutDesc)
{
	pOutDesc->AlphaToCoverageEnable = FALSE;
	pOutDesc->IndependentBlendEnable = TRUE;
	if (g_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		pOutDesc->IndependentBlendEnable = FALSE;
	}
	

	for (DWORD i=0; i<8; i++)
	{
		
		pOutDesc->RenderTarget[i].BlendEnable = FALSE;
		pOutDesc->RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		pOutDesc->RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		pOutDesc->RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;

		pOutDesc->RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		pOutDesc->RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		pOutDesc->RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		pOutDesc->RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
}
void SetBlendValueColorWriteDisable(D3D11_BLEND_DESC* pOutDesc)
{
	SetDefaultBlendValue(pOutDesc);

	for (DWORD i=0; i<8; i++)
	{
		pOutDesc->RenderTarget[i].RenderTargetWriteMask = 0;
	}
}

void SetDefaultSamplerValue(D3D11_SAMPLER_DESC* pOutDesc)
{
		pOutDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    pOutDesc->AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pOutDesc->AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pOutDesc->AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pOutDesc->MipLODBias = -1.0f;
	pOutDesc->MaxAnisotropy = 16;
	if (g_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		pOutDesc->MaxAnisotropy = 2;
	}
	pOutDesc->ComparisonFunc = D3D11_COMPARISON_NEVER;
	pOutDesc->BorderColor[0] = 1.0f;
	pOutDesc->BorderColor[1] = 1.0f;
	pOutDesc->BorderColor[2] = 1.0f;
	pOutDesc->BorderColor[3] = 1.0f;
	pOutDesc->MinLOD = -FLT_MAX;
    pOutDesc->MaxLOD = D3D11_FLOAT32_MAX;

}
void SetDefaultDepthStencilValue(D3D11_DEPTH_STENCIL_DESC* pOutDesc)
{
	pOutDesc->DepthEnable = TRUE;
	pOutDesc->DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	pOutDesc->DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	
	pOutDesc->StencilEnable = FALSE;
	pOutDesc->StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	pOutDesc->StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	pOutDesc->FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pOutDesc->FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;;

	pOutDesc->BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pOutDesc->BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	pOutDesc->BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

}

void OutputD3DErrorMsg(HRESULT hr,WCHAR* wchMsg)
{
	WCHAR	txt[512] = {0};
	
	WCHAR*	errMsg = L"Unknown";
	switch (hr)
	{
	case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
		errMsg = L"D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
		break;

	case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
		errMsg = L"D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
		break;
		
	case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
		errMsg = L"D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
		break;
			
	case DXGI_DDI_ERR_WASSTILLDRAWING:
		errMsg = L"DXGI_DDI_ERR_WASSTILLDRAWING";
		break;
		
	case E_FAIL:
		errMsg = L"E_FAIL";
		break;

	case E_INVALIDARG:
		errMsg = L"E_INVALIDARG";
		break;
		
	case E_OUTOFMEMORY:
		errMsg = L"E_OUTOFMEMORY";
		break;
		
	case DXGI_ERROR_DEVICE_REMOVED:
		errMsg = L"DXGI_ERROR_DEVICE_REMOVED";
		break;
	case DXGI_ERROR_INVALID_CALL:
		errMsg = L"DXGI_ERROR_INVALID_CALL";
		break;
	
	};
	
	swprintf_s(txt,L"%s - %s\n",wchMsg,errMsg);

	OutputDebugStringW(txt);
	__debugbreak();
}
	
void SetFeatureLevel(D3D_FEATURE_LEVEL featureLevel)
{
	g_FeatureLevel = featureLevel;
}
