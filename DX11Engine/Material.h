#pragma once

#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11ShaderResourceView* tex, ID3D11SamplerState* ss);
	~Material();

	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
	ID3D11ShaderResourceView* getTexture();
	ID3D11SamplerState* getSampleState();

	// Texture Info
	ID3D11ShaderResourceView* texture;
	ID3D11SamplerState* sampleState;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};

