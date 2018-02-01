#include "Camera.h"


Camera::Camera(float width, float height)
{
	position = XMFLOAT3(0, 0, -5);
	direction = XMFLOAT3(0, 0, 1);
	xRotation = 0;
	yRotation = 0;

	GenerateProjection(width, height);
}

Camera::~Camera()
{
}

void Camera::Update(float deltaTime)
{
	XMVECTOR dir = XMLoadFloat3(&direction);
	XMVECTOR forward = XMVectorSet(0, 0, 1, 0);

	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(xRotation, yRotation, 0);
	dir = XMVector3Rotate(forward, quat);
	XMStoreFloat3(&direction, dir);

	CheckInput(deltaTime);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR pos = XMLoadFloat3(&position);

	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(XMMatrixLookToLH(pos, dir, up)));
}

void Camera::MouseRotate(float x, float y)
{
	xRotation += x;
	yRotation += y;

	xRotation = fmod(xRotation, 2);
	yRotation = fmod(yRotation, 2);
}

void Camera::GenerateProjection(float width, float height)
{
	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(P)); // Transpose for HLSL!
}

void Camera::CheckInput(float deltaTime)
{
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR dir = XMLoadFloat3(&direction);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR left = XMVector3Cross(dir, up);

	float speed = 2.0f * deltaTime;

	if (GetAsyncKeyState('W') & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(dir, speed));
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(dir, -speed));
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(left, -speed));
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(left, speed));
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(up, speed));
	}
	if (GetAsyncKeyState('X') & 0x8000) {
		pos = XMVectorAdd(pos, XMVectorScale(up, -speed));
	}

	XMStoreFloat3(&position, pos);
}
