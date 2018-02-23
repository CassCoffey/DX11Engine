#pragma once
#include <DirectXMath.h>

#include "GameObject.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Vertex.h"

struct LightShaderInfo
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Vector;
	float Range;
};

class Light : public GameObject
{
public:
	Light(XMFLOAT4X4 iWorld, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
	~Light();

	virtual void PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth) {};
	virtual void Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth);

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11SamplerState* sampler;
	Mesh* mesh;
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	ID3D11ShaderResourceView* skybox;
	LightShaderInfo info;
	ID3D11RasterizerState* lightRasterState;
};

