#pragma once

#include "SimpleShader.h"

// For the DirectX Math library
using namespace DirectX;

class Material
{
public:
	Material(SimpleVertexShader* vs, SimplePixelShader* ps, XMFLOAT4 col, XMFLOAT2 scale, ID3D11ShaderResourceView* tex, ID3D11ShaderResourceView* norm, ID3D11ShaderResourceView* rough, ID3D11ShaderResourceView* met, ID3D11ShaderResourceView* sky, ID3D11SamplerState* ss);
	~Material();

	void PrepareMaterial(XMFLOAT4X4 proj, XMFLOAT4X4 view, XMFLOAT4X4 world);

private:
	// Basic Info
	XMFLOAT2 uvScale;
	XMFLOAT4 color;
	float shininess;

	// Texture Info
	ID3D11ShaderResourceView* texture;
	ID3D11ShaderResourceView* normals;
	ID3D11ShaderResourceView* roughness;
	ID3D11ShaderResourceView* metal;
	ID3D11ShaderResourceView* skybox;
	ID3D11SamplerState* sampleState;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};

