#pragma once

#include <d3d11.h>
#include "math.inl"

void SetFeatureLevel(D3D_FEATURE_LEVEL featureLevel);
HRESULT CompileShaderFromFile(char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut,DWORD ShaderParams,SYSTEMTIME* pOutLastWriteTime);

void SetDefaultReasterizeValue(D3D11_RASTERIZER_DESC* pOutDesc);
void SetDefaultBlendValue(D3D11_BLEND_DESC* pOutDesc);
void SetBlendValueColorWriteDisable(D3D11_BLEND_DESC* pOutDesc);
void SetDefaultSamplerValue(D3D11_SAMPLER_DESC* pOutDesc);
void SetDefaultDepthStencilValue(D3D11_DEPTH_STENCIL_DESC* pOutDesc);

void OutputD3DErrorMsg(HRESULT hr,WCHAR* wchMsg);


