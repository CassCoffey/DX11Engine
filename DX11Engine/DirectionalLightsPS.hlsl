Texture2D normalBuffer : register(t0);
Texture2D depthBuffer : register(t1);
TextureCube skyTexture : register(t2);
SamplerState basicSampler : register(s0);

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float4 screenPos	: TEXCOORD0;
	float2 uv			: TEXCOORD1;
};

struct DirLight
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 direction;
	float range;
};

// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	DirLight light;
	float3 CameraPosition;
	matrix inView;
	matrix inProjection;
};

float4 calculateDirLight(float3 normal, DirLight dLight)
{
	float3 normDir = normalize(-dLight.direction);

	float amount = saturate(dot(normal, normDir));

	return (dLight.diffuseColor * amount) + dLight.ambientColor;
}

float4 SkyboxReflection(float3 normal, float3 worldPos)
{
	float3 dirToCamera = normalize(CameraPosition - worldPos);

	float3 skyRefl = reflect(-dirToCamera, normal);
	float4 reflColor = skyTexture.Sample(basicSampler, skyRefl);

	return reflColor;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 screen;
screen.x = input.screenPos.x / input.screenPos.w / 2.0f + 0.5f;
screen.y = -input.screenPos.y / input.screenPos.w / 2.0f + 0.5f;

float4 lightColor = calculateDirLight(normalBuffer.Sample(basicSampler, screen), light);

return lightColor;
}