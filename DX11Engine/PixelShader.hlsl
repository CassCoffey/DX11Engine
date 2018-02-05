Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
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
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
};

struct DirectionalLight
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 direction;
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
	DirectionalLight light;
	PointLight lightTwo;

	float3 CameraPosition;
};

float4 calculateDirLight(float3 normal, DirectionalLight dLight)
{
	float3 normDir = normalize(-dLight.direction);

	float amount = saturate(dot(normal, normDir));

	return (dLight.diffuseColor * amount) + dLight.ambientColor;
}

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

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 unpackedNormal = normalTexture.Sample(basicSampler, input.uv) * 2.0f - 1.0f;

	float3 N = normalize(input.normal);
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	float3 finalNormal = mul(unpackedNormal, TBN);

	float4 lightOneColor = calculateDirLight(finalNormal, light);
	float4 lightTwoColor = calculatePointLight(finalNormal, input.worldPos, lightTwo);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	return lerp(surfaceColor, SkyboxReflection(finalNormal, input.worldPos), 0.5f) * float4(float3(0.1f,0.1f,0.1f) + lightOneColor + lightTwoColor, 1);
}