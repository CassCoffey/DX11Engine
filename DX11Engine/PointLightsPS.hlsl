Texture2D albedoBuffer : register(t0);
Texture2D normalBuffer : register(t1);
Texture2D pbrBuffer : register(t2);
Texture2D depthBuffer : register(t3);
TextureCube skyTexture : register(t4);
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

struct PointLight
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 position;
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
	PointLight light;
	float3 CameraPosition;
	matrix inView;
	matrix inProjection;
};

// Range-based attenuation function
float Attenuate(PointLight light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);

	// Ranged-based attenuation
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));

	// Soft falloff
	return att * att;
}

// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// Also slide 65 of http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
static const float F0_NON_METAL = 0.04f;

static const float MIN_ROUGHNESS = 0.0000001f;

static const float PI = 3.14159265359f;

float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}

float SpecDistribution(float3 n, float3 h, float roughness)
{
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS);

	float denomToSquare = NdotH2 * (a2 - 1) + 1;

	return a2 / (PI * denomToSquare * denomToSquare);
}

float3 Fresnel(float3 v, float3 h, float3 f0)
{
	float VdotH = saturate(dot(v, h));

	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

float GeometricShadowing(float3 n, float3 v, float3 h, float roughness)
{
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	return NdotV / (NdotV * (1 - k) + k);
}

float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float metalness, float3 specColor)
{
	float3 h = normalize(v + l);

	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, h, roughness) * GeometricShadowing(n, l, h, roughness);

	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}

float3 DiffuseEnergyConserve(float diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

float3 WorldPosFromDepth(float depth, float2 texCoord)
{
	float z = depth;

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
	
	// Calc light direction
	float3 toLight = normalize(light.position - worldPos);
	float3 toCam = normalize(CameraPosition - worldPos);
	
	float3 specColor = lerp(F0_NON_METAL.rrr, albedoBuffer.Sample(basicSampler, screen).rgb, pbrBuffer.Sample(basicSampler, screen).g);

	// Calculate the light amounts
	float atten = Attenuate(light, worldPos);
	float diff = DiffusePBR(normalBuffer.Sample(basicSampler, screen), toLight);
	float3 spec = MicrofacetBRDF(normalBuffer.Sample(basicSampler, screen), toLight, toCam, pbrBuffer.Sample(basicSampler, screen).r, pbrBuffer.Sample(basicSampler, screen).g, specColor);
	
	// Calculate diffuse with energy conservation
	// (Reflected light doesn't diffuse)
	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, pbrBuffer.Sample(basicSampler, screen).g);
	
	// Combine
	return float4((balancedDiff + spec) * atten * light.diffuseColor, 1);
}