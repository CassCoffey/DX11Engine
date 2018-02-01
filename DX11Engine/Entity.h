#pragma once

#include "GameObject.h"
#include "Vertex.h"
#include "Mesh.h"

// For the DirectX Math library
using namespace DirectX;

class Entity : public GameObject
{
public:
	Material* material;
	Mesh* mesh;

	Entity(Mesh* iMesh, Material* iMaterial, XMFLOAT4X4 iWorld, XMFLOAT3 iPos, XMFLOAT3 iRot, XMFLOAT3 iScale);
	~Entity();

	void PrepareMaterial(XMFLOAT4X4 proj, XMFLOAT4X4 view);
	void Draw(ID3D11DeviceContext* context, XMFLOAT4X4 proj, XMFLOAT4X4 view);
};

