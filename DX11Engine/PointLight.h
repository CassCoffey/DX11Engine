#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "GameObject.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Vertex.h"

class PointLight : public GameObject
{
public:
	PointLight(XMFLOAT4X4 iWorld, Mesh* iMesh, SimpleVertexShader* vs, SimplePixelShader* ps, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
	~PointLight();

	void PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* worldPos);
	void Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* worldPos);

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	Mesh* mesh;
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 position;
};

