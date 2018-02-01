#include "GameObject.h"

GameObject::GameObject(XMFLOAT4X4 iWorld, XMFLOAT3 iPos, XMFLOAT3 iRot, XMFLOAT3 iScale)
{
	worldMatrix = iWorld;
	position = iPos;
	rotation = iRot;
	scale = iScale;
}

GameObject::~GameObject()
{
}

XMFLOAT4X4 GameObject::GetMatrix()
{
	return worldMatrix;
}

XMFLOAT3 GameObject::GetPosition()
{
	return position;
}

XMFLOAT3 GameObject::GetRotation()
{
	return rotation;
}

XMFLOAT3 GameObject::GetScale()
{
	return scale;
}

void GameObject::SetMatrix(XMFLOAT4X4 newWorld)
{
	worldMatrix = newWorld;
}

void GameObject::SetPosition(XMFLOAT3 newPosition)
{
	position = newPosition;
}

void GameObject::SetRotation(XMFLOAT3 newRotation)
{
	rotation = newRotation;
}

void GameObject::SetScale(XMFLOAT3 newScale)
{
	scale = newScale;
}

void GameObject::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
}

void GameObject::SetRotation(float x, float y, float z)
{
	rotation = XMFLOAT3(x, y, z);
}

void GameObject::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
}

void GameObject::Move(XMFLOAT3 translation)
{
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR trans = XMLoadFloat3(&translation);

	pos = XMVectorAdd(pos, trans);

	XMStoreFloat3(&position, pos);
}

void GameObject::Move(float x, float y, float z)
{
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR trans = XMLoadFloat3(&XMFLOAT3(x, y, z));

	pos = XMVectorAdd(pos, trans);

	XMStoreFloat3(&position, pos);
}

void GameObject::Draw(ID3D11DeviceContext* context, XMFLOAT4X4 proj, XMFLOAT4X4 view)
{
	UpdateWorldMatrix();
}

void GameObject::UpdateWorldMatrix()
{
	XMVECTOR pos = XMLoadFloat3(&position);
	XMMATRIX posMat = XMMatrixTranslationFromVector(pos);
	XMVECTOR rot = XMLoadFloat3(&rotation);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYawFromVector(rot);
	XMVECTOR sca = XMLoadFloat3(&scale);
	XMMATRIX scaMat = XMMatrixScalingFromVector(sca);

	XMMATRIX world = XMMatrixIdentity();

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(scaMat * rotMat * posMat));
}