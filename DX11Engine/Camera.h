#pragma once

#include "Vertex.h"
#include <d3d11.h>

// For the DirectX Math library
using namespace DirectX;

class Camera
{
public:
	Camera(float width, float height);
	~Camera();

	void Update(float deltaTime);
	void MouseRotate(float x, float y);
	void GenerateProjection(float width, float height);

	XMFLOAT4X4 viewMat;
	XMFLOAT4X4 projMat;

	XMFLOAT3 position;
	XMFLOAT3 direction;
	float xRotation;
	float yRotation;

private:
	void CheckInput(float deltaTime);
};

