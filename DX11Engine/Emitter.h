#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "Camera.h"
#include "SimpleShader.h"

struct Particle
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 StartPosition;
	DirectX::XMFLOAT3 StartVelocity;
	float Size;
	float Rotation;
	float RotationalVelocity;
	float Age;
};

struct ParticleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 Color;
	float Size;
	float Rotation;
};

class Emitter
{
public:
	Emitter(
		int maxParticles,
		int burstsPerSecond,
		int particlesPerBurst,
		float emitTime,
		float lifetime,
		float startSize,
		float endSize,
		float minRotation,
		float maxRotation,
		float minRotVel,
		float maxRotVel,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 maxStartVelocity,
		DirectX::XMFLOAT3 minStartVelocity,
		DirectX::XMFLOAT3 emitterPosition,
		DirectX::XMFLOAT3 emitterAccelertaion,
		ID3D11Device* device,
		SimpleVertexShader* vs,
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture
	);
	~Emitter();

	void Update(float dt);

	void UpdateSingleParticle(float dt, int index);
	void SpawnParticle();

	void ResetParticle(int index);
	void ResetEmitter();
	void MoveEmitter(float x, float y, float z);

	void CopyParticlesToGPU(ID3D11DeviceContext* context);
	void CopyOneParticle(int index);
	void Draw(ID3D11DeviceContext* context, Camera* camera);

private:
	// Emission stuff
	int burstsPerSecond;
	int particlesPerBurst;
	float secondsPerBurst;
	float timeSinceEmit;
	float totalTimeSinceEmit;

	int livingParticleCount;
	float emitTime;
	float lifetime;
	DirectX::XMFLOAT3 emitterAcceleration;
	DirectX::XMFLOAT3 emitterPosition;
	DirectX::XMFLOAT3 minStartVelocity;
	DirectX::XMFLOAT3 maxStartVelocity;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float startSize;
	float endSize;
	float minRotation;
	float maxRotation;
	float minRotVel;
	float maxRotVel;

	// Particle Array
	Particle* particles;
	int maxParticles;
	int firstDeadIndex;
	int firstAliveIndex;

	// Rendering
	ParticleVertex* localParticleVertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* texture;
	SimpleVertexShader* vs;
	SimplePixelShader* ps;
};

