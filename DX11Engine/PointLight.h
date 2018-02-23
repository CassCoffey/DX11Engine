#pragma once
#include <DirectXMath.h>

#include "Light.h"

class PointLight : public Light
{
public:
	PointLight(XMFLOAT4X4 iWorld, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, float iRange);
	~PointLight();

	void PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth);

	float range;
};

