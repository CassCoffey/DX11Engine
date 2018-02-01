// Constant buffer for C++ data being passed in
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

// Describes individual vertex data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float4 color		: COLOR;
	float size : SIZE;
	float rotation : ROTATION;
};

// Defines the output data of our vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float4 color		: TEXCOORD1;
};

float2x2 GetRotationMatrix(float rotation)
{
	float c = cos(rotation);
	float s = sin(rotation);

	return float2x2(c, -s, s, c);
}

// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Calculate output position
	matrix viewProj = mul(view, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);

	// Use UV to offset position (billboarding)
	float2 offset = input.uv * 2 - 1;
	offset *= input.size;
	offset.y *= -1;
	float2x2 rotMat = GetRotationMatrix(input.rotation);
	offset = mul(rotMat, offset);

	output.position.xy += offset;

	// Pass uv through
	output.uv = input.uv;
	output.color = input.color;

	return output;
}