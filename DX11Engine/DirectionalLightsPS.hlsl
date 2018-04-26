Texture2D albedoBuffer : register(t0);
Texture2D normalBuffer : register(t1);
Texture2D roughnessBuffer : register(t2);
Texture2D metalBuffer : register(t3);
Texture2D depthBuffer : register(t4);
TextureCube skyTexture : register(t5);
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

// === PHYSICALLY BASED LIGHTING ====================================

// PBR Constants:

// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// Also slide 65 of http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
static const float F0_NON_METAL = 0.04f;

// Need a minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

static const float PI = 3.14159265359f;

// Lambert diffuse BRDF - Same as the basic lighting!
float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}



// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

										  // ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}



// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}



// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
//
// n - Normal
// v - View vector
//
// G(l,v,h)
float GeometricShadowing(float3 n, float3 v, float3 h, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}



// Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
//
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float metalness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor); // This ranges from F0_NON_METAL to actual specColor based on metalness
	float G = GeometricShadowing(n, v, h, roughness) * GeometricShadowing(n, l, h, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}

// Calculates diffuse color based on energy conservation
//
// diffuse - Diffuse color
// specular - Specular color (including light color)
// metalness - surface metalness amount
//
// Metals should have an albedo of (0,0,0)...mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
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

	// Get normalize direction to the light
	float3 toLight = normalize(-light.direction);
	float3 toCam = normalize(CameraPosition - worldPos);

	float3 specColor = lerp(F0_NON_METAL.rrr, albedoBuffer.Sample(basicSampler, screen).rgb, metalBuffer.Sample(basicSampler, screen).r);

	// Calculate the light amounts
	float diff = DiffusePBR(normalBuffer.Sample(basicSampler, screen), toLight);
	float3 spec = MicrofacetBRDF(normalBuffer.Sample(basicSampler, screen), toLight, toCam, roughnessBuffer.Sample(basicSampler, screen).r, metalBuffer.Sample(basicSampler, screen).r, specColor);

	// Calculate diffuse with energy conservation
	// (Reflected light doesn't get diffused)
	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalBuffer.Sample(basicSampler, screen).r);

	// Combine amount with 
	return float4((balancedDiff + spec) * light.diffuseColor, 1);
}