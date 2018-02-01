#pragma once

#include "SimpleShader.h"
#include "Material.h"
#include <d3d11.h>

// For the DirectX Math library
using namespace DirectX;

class GameObject
{
public:
	// UI Stuff
	ID3D11ShaderResourceView* texture;

	XMFLOAT4X4 worldMatrix;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;

	GameObject(XMFLOAT4X4 iWorld, XMFLOAT3 iPos, XMFLOAT3 iRot, XMFLOAT3 iScale);
	~GameObject();

	XMFLOAT4X4 GetMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();
	XMFLOAT3 GetScale();

	void SetMatrix(XMFLOAT4X4 newWorld);
	void SetPosition(XMFLOAT3 newPosition);
	void SetRotation(XMFLOAT3 newRotation);
	void SetScale(XMFLOAT3 newScale);

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	void Move(XMFLOAT3 translation);
	void Move(float x, float y, float z);

	void Draw(ID3D11DeviceContext* context, XMFLOAT4X4 proj, XMFLOAT4X4 view);

	void UpdateWorldMatrix();
};

