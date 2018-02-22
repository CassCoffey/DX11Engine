#pragma once
#include <DirectXMath.h>

#include "GameObject.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Vertex.h"

struct PointLightShaderInfo
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Position;
	float Range;
};

class PointLight : public GameObject
{
public:
	PointLight(XMFLOAT4X4 iWorld, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, float iRange);
	~PointLight();

	void PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth);
	void Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth);

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11SamplerState* sampler;
	Mesh* mesh;
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	float range;
	ID3D11ShaderResourceView* skybox;
	PointLightShaderInfo info;
};

