// Globals
Texture2D	gTexture : register(t0);

// Constants
cbuffer ConstantBuffer {
	float	mOpacity;
};

// SamplerState
SamplerState TextureSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

// PixelShaderInput
struct PixelShaderInput {
	float4	mPosition : SV_POSITION;
	float2	mTextureCoordinate : TEXCOORD0;
	float	mClipDistance : SV_ClipDistance0;
};

// Function
float4 main(PixelShaderInput input) : SV_TARGET
{
	// Setup
	float	width, height;
	gTexture.GetDimensions(width, height);

	float4	color =
					gTexture.Sample(TextureSampler,
							float2(input.mTextureCoordinate.x / width, input.mTextureCoordinate.y / height));
	color.a *= mOpacity;

	return color;
}