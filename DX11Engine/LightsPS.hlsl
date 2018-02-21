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
	float3 worldPos		: POSITION;
};

struct PointLight
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 position;
};

// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	PointLight light;
	float3 CameraPosition;
	matrix inView;
	matrix inProjection;
};

float4 calculatePointLight(float3 normal, float3 worldPos, PointLight pLight)
{
	float3 dirToPointLight = normalize(pLight.position - worldPos);

	float amount = saturate(dot(normal, dirToPointLight));

	float3 refl = reflect(-dirToPointLight, normal);

	float3 dirToCamera = normalize(CameraPosition - worldPos);

	float specular = pow(saturate(dot(refl, dirToCamera)), 64);

	return (pLight.diffuseColor * amount) + pLight.ambientColor + specular;
}

float4 SkyboxReflection(float3 normal, float3 worldPos)
{
	float3 dirToCamera = normalize(CameraPosition - worldPos);

	float3 skyRefl = reflect(-dirToCamera, normal);
	float4 reflColor = skyTexture.Sample(basicSampler, skyRefl);

	return reflColor;
}

float3 WorldPosFromDepth(float depth, float2 texCoord) 
{
	float z = depth * 2.0 - 1.0;

	float4 clipSpacePosition = float4(texCoord.x * 2.0 - 1.0, -(texCoord.y * 2.0 - 1.0), z, 1.0);
	float4 viewSpacePosition = mul(clipSpacePosition, inProjection);

	viewSpacePosition /= viewSpacePosition.w;

	float4 worldSpacePosition = mul(viewSpacePosition, inView);

	return worldSpacePosition.xyz;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 screen;
	screen.x = input.screenPos.x / input.screenPos.w / 2.0f + 0.5f;
	screen.y = -input.screenPos.y / input.screenPos.w / 2.0f + 0.5f;

	float3 worldPos = WorldPosFromDepth(depthBuffer.Load(uint3(input.position.xy, 0)), screen);

	float4 lightColor = calculatePointLight(normalBuffer.Sample(basicSampler, screen), worldPos, light);

	return float4(worldPos, 1);
}