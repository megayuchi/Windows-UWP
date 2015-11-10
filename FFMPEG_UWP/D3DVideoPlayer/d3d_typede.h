#pragma once

#include <d3d11.h>


#pragma pack(push,1)

union RGBA
{
	struct
	{
		BYTE	r;
		BYTE	g;
		BYTE	b;
		BYTE	a;
	};
	BYTE		bColorFactor[4];
};
#pragma pack(pop)

struct D3DTVERTEX
{
	float		x;
	float		y;
	float		z;
	
	float		u;
	float		v;

};
struct TVERTEX
{
	float u;
	float v;
};

union COLOR_VALUE
{
	struct 
	{
		float r;
		float g;
		float b;
		float a;
	};
	float	rgba[4];
	void Set(float rr,float gg,float bb,float aa) {r = rr; g = gg; b = bb; a = aa;}
	void Set(DWORD dwColor)
	{
		r = (float)((dwColor & 0x00ff0000) >> 16) / 255.0f;		// R
		g = (float)((dwColor & 0x0000ff00) >> 8) / 255.0f;		// G
		b = (float)((dwColor & 0x000000ff)) / 255.0f;			// B;
		a = (float)((dwColor & 0xff000000) >> 24) / 255.0f;		// A
	}
};


enum BLEND_TYPE
{
	BLEND_TYPE_TRANSP	= 0,
	BLEND_TYPE_ADD		= 1,
	BLEND_TYPE_NO_COLOR	= 2
};
#define BLEND_TYPE_NUM	3
/*
enum RASTERIZE_TYPE
{
	RASTERIZE_TYPE_CULL_DEFAULT			= 0,
	RASTERIZE_TYPE_CULL_BACK			= 0,
	RASTERIZE_TYPE_CULL_NONE			= 1,
	RASTERIZE_TYPE_CULL_BACK_WIREFRAME	= 2
};
*/
#define RASTER_CULL_MASK	0x03	// 0011
#define RASTER_FILL_MASK	0x04	// 0100

inline DWORD	GetRasterTypeIndex(D3D11_CULL_MODE cullMode,D3D11_FILL_MODE fillMode)
{
	DWORD dwIndex = (cullMode - D3D11_CULL_NONE) | ((fillMode - D3D11_FILL_WIREFRAME) << 2);
	return dwIndex;
}

#define MAX_RASTER_TYPE_NUM	8

#define DEPTH_WRITE_MASK	0x01
#define DEPTH_TEST_MASK		0x02

inline DWORD	GetDepthTypeIndex(BOOL bDisableWrite,BOOL bDisableTest)
{
	DWORD	dwIndex = ( (bDisableTest<<1) | bDisableWrite ) & 0x00000003;
	return dwIndex;

}
#define MAX_DEPTH_TYPE_NUM	4

enum SAMPLER_TYPE
{
	SAMPLER_TYPE_WRAP_POINT,
	SAMPLER_TYPE_WRAP_LINEAR,
	SAMPLER_TYPE_WRAP_ANISOTROPIC,

	SAMPLER_TYPE_CLAMP_POINT,
	SAMPLER_TYPE_CLAMP_LINEAR,
	SAMPLER_TYPE_CLAMP_ANISOTROPIC,

	SAMPLER_TYPE_MIRROR_POINT,
	SAMPLER_TYPE_MIRROR_LINEAR,
	SAMPLER_TYPE_MIRROR_ANISOTROPIC,
	
	SAMPLER_TYPE_BORDER_POINT,
	SAMPLER_TYPE_BORDER_LINEAR,
	SAMPLER_TYPE_BORDER_ANISOTROPIC
};
#define MAX_SAMPLER_TYPE_NUM	12

#define		MAX_SHADER_NAME_LEN				63
#define		MAX_SHADER_NAME_BUFFER_LEN		64
#define		MAX_SHADER_NUM					1024
#define		MAX_CODE_SIZE					(1024*512)


enum SHADER_TYPE
{
	SHADER_TYPE_VERTEX_SHADER	= 0,
	SHADER_TYPE_PIXEL_SHADER	= 1,
	SHADER_TYPE_HULL_SHADER		= 2,
	SHADER_TYPE_DOMAIN_SHADER	= 3,
	SHADER_TYPE_GEOMETRY_SHADER	= 4,
	SHADER_TYPE_COMPUTE_SHADER	= 5
};

struct SHADER_HANDLE
{
	IUnknown*				pD3DShader;
	DWORD					dwRefCount;
	BOOL					bPreLoaded;
	SYSTEMTIME				CreationTime;
	SHADER_TYPE				ShaderType;
	DWORD					dwCodeSize;
	DWORD					dwShaderFileNameLen;
	DWORD					dwShaderNameLen;
	char					szShaderFileName[MAX_SHADER_NAME_BUFFER_LEN];
	char					szShaderName[MAX_SHADER_NAME_BUFFER_LEN];
	

	DWORD					pCodeBuffer[1];
};


struct CONSTANT_BUFFER_SPRITE
{
	float			render_pos_x;
	float			render_pos_y;
	float			render_width;
	float			render_height;

	float			screen_width;
	float			screen_height;
	float			render_z;
	float			fAlpha;
	
	float			u_offset;
	float			v_offset;
	float			reseved0;
	float			reseved1;

		
	COLOR_VALUE		diffuseColor;
};