// Texture Variables
Texture2D diffuseTexture	: register(t0);
Texture2D normalTexture		: register(t1);
Texture2D roughnessTexture	: register(t2);
Texture2D metalTexture		: register(t3);
SamplerState basicSampler	: register(s0);

// Data that can change per material
cbuffer perMaterial : register(b0)
{
	// Surface color
	float4 Color;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD0;
	float depth			: TEXCOORD1;
};

struct PSOutput
{
	float4 color	: SV_TARGET0;
	float4 normals	: SV_TARGET1;
	float4 roughness: SV_TARGET2;
	float4 metal	: SV_TARGET3;
	float4 depth	: SV_TARGET4;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
PSOutput main(VertexToPixel input)
{
	PSOutput OUT;

	float3 unpackedNormal = normalTexture.Sample(basicSampler, input.uv) * 2.0f - 1.0f;

	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	float3 finalNormal = mul(unpackedNormal, TBN);

	OUT.normals = float4(finalNormal, 1);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	OUT.color = surfaceColor;

	float roughness = roughnessTexture.Sample(basicSampler, input.uv).r;
	OUT.roughness = roughness;

	float metal = metalTexture.Sample(basicSampler, input.uv).r;
	OUT.metal = metal;

	OUT.depth = float4(input.depth.x, input.depth.x, input.depth.x, 1);

	return OUT;
}