

cbuffer ConstantBufferSprite : register( b0 )
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

	float4			diffuseColor;
}

Texture2D		texDiffuse		: register( t0 );
SamplerState	samplerDiffuse	: register( s0 );

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4		Pos			 : POSITION;
	float2		TexCoord	 : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos		 : SV_POSITION;
    float4 Color	 : COLOR;
	float2 TexCoord	 : TEXCOORD0;
	float4 TexCoordSide[4] : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

float4 CalcPos(float4 inputPos)
{
	// position
	float	scale_x = render_width / screen_width;
	float	scale_y = render_height / screen_height;

	float	x_offset = render_pos_x / screen_width;
	float	y_offset = render_pos_y / screen_height;

	
	float2	pos = inputPos.xy * float2(scale_x,scale_y) + float2(x_offset,y_offset);
	float4	outputPos = float4(pos.x*2-1 ,(1-pos.y)*2-1,render_z,1);
	
	return outputPos;

}
PS_INPUT vsDefault( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	
	
	output.Pos = CalcPos(input.Pos);

	//output.Pos = input.Pos * float4(w,h,render_z,1);
	output.TexCoord = input.TexCoord;
	output.Color = diffuseColor;

	// 빌어먹을 DX Feature Level 9.x는 쉐이더 아웃풋을 9개 이상 지원하지 못하므로 이런 뻘짓을 한다...-_-;
	output.TexCoordSide[0].zw = input.TexCoord.xy + float2(-u_offset,-v_offset);	 // 왼쪽 위
	output.TexCoordSide[1].zw = input.TexCoord.xy + float2(u_offset,-v_offset);	 // 오른쪽 위
	output.TexCoordSide[2].zw = input.TexCoord.xy + float2(-u_offset,v_offset);	 // 왼쪽 아래
	output.TexCoordSide[3].zw = input.TexCoord.xy + float2(u_offset,v_offset);	 // 오른쪽 아래

	output.TexCoordSide[0].xy = input.TexCoord.xy + float2(0,-v_offset);			 // 위
	output.TexCoordSide[1].xy = input.TexCoord.xy + float2(-u_offset,0);			 // 왼쪽
	output.TexCoordSide[2].xy = input.TexCoord.xy + float2(u_offset,0);			 // 오른쪽
	output.TexCoordSide[3].xy = input.TexCoord.xy + float2(0,v_offset);			 // 아래

    return output;
}
float3 GetRGBFromYUV(float4 texColor)
{
	//float	Y = texColor.r;
	//float	U = texColor.g;
	//float	V = texColor.b;

	/*
	BYTE R = (BYTE)( (float)Y + 1.402f*(float)(V-128));
	BYTE G = (BYTE)( (float)Y - 0.344f*(float)(U-128) - 0.714f*(float)(V-128) );
	BYTE B = (BYTE)( (float)Y + 1.772f*(float)(U-128) );
	*/


	float	Y = 1.1643 * texColor.r;
	float	U = texColor.g - 0.5;
	float	V = texColor.b - 0.5;

	float3	color;
	color.r = Y + 1.5958 * V;
	color.g = Y - 0.39173 * U - 0.81290 * V;
	color.b = Y + 2.017 * U;

	return color;
}

float4 psDefault( PS_INPUT input) : SV_Target
{
	float4	texColor = texDiffuse.Sample( samplerDiffuse, input.TexCoord);
	
	float3	color = GetRGBFromYUV(texColor);
	float4	outColor = input.Color * float4(color,1);
	
	return outColor;
}

float4 psGrey( PS_INPUT input) : SV_Target
{
	float4	texColor = texDiffuse.Sample( samplerDiffuse, input.TexCoord);
	
	float3	color = GetRGBFromYUV(texColor);
	
	float3	bwConst = float3(0.3f,0.59f,0.11f);
	float	bw = dot(color,bwConst);
	
	float4	outColor = float4(bw,bw,bw,1);
	
	return outColor;
}

	
float4 psBlur( PS_INPUT input) : SV_Target
{
	float4	texColor = texDiffuse.Sample(samplerDiffuse, input.TexCoord);
	float3	Center = GetRGBFromYUV(texColor);
	
	float3	ColorSum = Center;

	// 주변 점(8픽셀)의 합
	int i;
	for (i=0; i<4; i++)
	{
		float4	texColor = texDiffuse.Sample(samplerDiffuse,input.TexCoordSide[i].xy);
		ColorSum += GetRGBFromYUV(texColor);
	}	
	for (i=0; i<4; i++)
	{
		float4	texColor = texDiffuse.Sample(samplerDiffuse,input.TexCoordSide[i].zw);
		ColorSum += GetRGBFromYUV(texColor);
	}	
	float4	outtColor = float4(ColorSum * (1.0f / 9.0f),1);

	return outtColor;
}

float4 psEdge( PS_INPUT input) : SV_Target
{
	float4	texColor = texDiffuse.Sample(samplerDiffuse, input.TexCoord);
	float3	Center = GetRGBFromYUV(texColor);
	
	//-1, -1, -1,
	//-1,  8  -1,	
	//-1, -1, -1

	float	mask[8] =
	{ 
		-1, -1, -1,
		-1,	    -1,	// center = 8
	    -1, -1, -1
	};
	float3	ColorSum = Center*8;

	int i;
	for (i=0; i<4; i++)
	{
		float4	texColor = texDiffuse.Sample(samplerDiffuse,input.TexCoordSide[i].xy);
		ColorSum += GetRGBFromYUV(texColor) * mask[i];
	}	
	for (i=0; i<4; i++)
	{
		float4	texColor = texDiffuse.Sample(samplerDiffuse,input.TexCoordSide[i].zw);
		ColorSum += GetRGBFromYUV(texColor) * mask[i+4];
	}	
	
	//float4	outtColor = float4(ColorSum * (1.0f / 9.0f),1);
	float4		edgeColor = float4(1,1,1,1) - float4(ColorSum,0);
	//float4		outColor = float4(Center,1) * edgeColor;

	return edgeColor;
}
