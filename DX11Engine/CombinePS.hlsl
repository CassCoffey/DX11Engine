Texture2D colorTexture : register(t0);
Texture2D lightTexture : register(t1);
SamplerState basicSampler : register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return colorTexture.Sample(basicSampler, input.uv) * lightTexture.Sample(basicSampler, input.uv);
}