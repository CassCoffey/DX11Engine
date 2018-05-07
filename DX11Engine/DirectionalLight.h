#pragma once
#include "Light.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight(XMFLOAT4X4 iWorld, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot);
	~DirectionalLight();

	void PrepareShader(Camera* camera, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* pbr, ID3D11ShaderResourceView* depth);
	void Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* pbr, ID3D11ShaderResourceView* depth);

	float range;
};

