#include "Emitter.h"

using namespace DirectX;

Emitter::Emitter(
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
	DirectX::XMFLOAT3 emitterAcceleration,
	ID3D11Device* device,
	SimpleVertexShader* vs,
	SimplePixelShader* ps,
	ID3D11ShaderResourceView* texture
)
{
	// Save params
	this->vs = vs;
	this->ps = ps;
	this->texture = texture;

	this->maxParticles = maxParticles;
	this->emitTime = emitTime;
	this->lifetime = lifetime;
	this->startColor = startColor;
	this->endColor = endColor;
	this->maxStartVelocity = maxStartVelocity;
	this->minStartVelocity = minStartVelocity;
	this->startSize = startSize;
	this->endSize = endSize;
	this->minRotation = minRotation;
	this->maxRotation = maxRotation;
	this->minRotVel = minRotVel;
	this->maxRotVel = maxRotVel;
	this->burstsPerSecond = burstsPerSecond;
	this->particlesPerBurst = particlesPerBurst;
	this->secondsPerBurst = 1.0f / burstsPerSecond;

	this->emitterPosition = emitterPosition;
	this->emitterAcceleration = emitterAcceleration;

	totalTimeSinceEmit = 0;
	timeSinceEmit = secondsPerBurst;
	livingParticleCount = 0;
	firstAliveIndex = 0;
	firstDeadIndex = 0;

	// Make particle array
	particles = new Particle[maxParticles];

	// Create particle vertices
	localParticleVertices = new ParticleVertex[4 * maxParticles];
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		localParticleVertices[i + 0].UV = XMFLOAT2(0, 0);
		localParticleVertices[i + 1].UV = XMFLOAT2(1, 0);
		localParticleVertices[i + 2].UV = XMFLOAT2(1, 1);
		localParticleVertices[i + 3].UV = XMFLOAT2(0, 1);
	}


	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	device->CreateBuffer(&vbDesc, 0, &vertexBuffer);

	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	delete[] indices;
}


Emitter::~Emitter()
{
	delete[] particles;
	delete[] localParticleVertices;
	vertexBuffer->Release();
	indexBuffer->Release();
}

void Emitter::Update(float dt)
{
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			UpdateSingleParticle(dt, i);
	}
	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
			UpdateSingleParticle(dt, i);
		for (int i = 0; i < firstDeadIndex; i++)
			UpdateSingleParticle(dt, i);
	}

	totalTimeSinceEmit += dt;
	timeSinceEmit += dt;

	while (((emitTime == 0) || (totalTimeSinceEmit < emitTime)) && timeSinceEmit > secondsPerBurst)
	{
		for (int i = 0; i < particlesPerBurst; i++)
		{
			SpawnParticle();
		}
		timeSinceEmit -= secondsPerBurst;
	}
}

void Emitter::UpdateSingleParticle(float dt, int index)
{
	if (particles[index].Age >= lifetime)
		return;

	particles[index].Age += dt;
	if (particles[index].Age >= lifetime)
	{
		// move alive count
		firstAliveIndex++;
		firstAliveIndex %= maxParticles;
		livingParticleCount--;
		return;
	}

	float agePercent = particles[index].Age / lifetime;

	particles[index].Rotation += dt * particles[index].RotationalVelocity;

	XMStoreFloat4(
		&particles[index].Color,
		XMVectorLerp(
			XMLoadFloat4(&startColor),
			XMLoadFloat4(&endColor),
			agePercent));

	particles[index].Size = startSize + agePercent * (endSize - startSize);

	XMVECTOR startPos = XMLoadFloat3(&particles[index].StartPosition);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].StartVelocity);
	XMVECTOR accel = XMLoadFloat3(&emitterAcceleration);
	float t = particles[index].Age;

	XMStoreFloat3(
		&particles[index].Position,
		accel * t * t / 2.0f + startVel * t + startPos);
}

void Emitter::ResetParticle(int index)
{
	float xVelocityRange = maxStartVelocity.x - minStartVelocity.x;
	float yVelocityRange = maxStartVelocity.y - minStartVelocity.y;
	float zVelocityRange = maxStartVelocity.z - minStartVelocity.z;
	float rRange = maxRotation - minRotation;
	float rVRange = maxRotVel - minRotVel;

	particles[index].Age = 0;
	particles[index].Size = startSize;
	particles[index].Color = startColor;
	particles[index].Position = emitterPosition;
	particles[index].Rotation = minRotation + (((float)rand() / RAND_MAX) * rRange);
	particles[index].RotationalVelocity = minRotVel + (((float)rand() / RAND_MAX) * rVRange);
	particles[index].StartPosition = emitterPosition;
	particles[index].StartVelocity = XMFLOAT3(0, 0, 0);
	particles[index].StartVelocity.x = minStartVelocity.x + (((float)rand() / RAND_MAX) * xVelocityRange);
	particles[index].StartVelocity.y = minStartVelocity.y + (((float)rand() / RAND_MAX) * yVelocityRange);
	particles[index].StartVelocity.z = minStartVelocity.z + (((float)rand() / RAND_MAX) * zVelocityRange);
}

void Emitter::ResetEmitter()
{
	totalTimeSinceEmit = 0;
	timeSinceEmit = secondsPerBurst;
}

void Emitter::MoveEmitter(float x, float y, float z)
{
	emitterPosition = XMFLOAT3(x, y, z);
}

void Emitter::SpawnParticle()
{
	if (livingParticleCount == maxParticles)
		return;

	ResetParticle(firstDeadIndex);

	firstDeadIndex++;
	firstDeadIndex %= maxParticles;

	livingParticleCount++;
}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext* context)
{
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}
	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
			CopyOneParticle(i);

		for (int i = 0; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localParticleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	context->Unmap(vertexBuffer, 0);
}

void Emitter::CopyOneParticle(int index)
{
	int i = index * 4;

	localParticleVertices[i + 0].Position = particles[index].Position;
	localParticleVertices[i + 1].Position = particles[index].Position;
	localParticleVertices[i + 2].Position = particles[index].Position;
	localParticleVertices[i + 3].Position = particles[index].Position;

	localParticleVertices[i + 0].Size = particles[index].Size;
	localParticleVertices[i + 1].Size = particles[index].Size;
	localParticleVertices[i + 2].Size = particles[index].Size;
	localParticleVertices[i + 3].Size = particles[index].Size;

	localParticleVertices[i + 0].Color = particles[index].Color;
	localParticleVertices[i + 1].Color = particles[index].Color;
	localParticleVertices[i + 2].Color = particles[index].Color;
	localParticleVertices[i + 3].Color = particles[index].Color;

	localParticleVertices[i + 0].Rotation = particles[index].Rotation;
	localParticleVertices[i + 1].Rotation = particles[index].Rotation;
	localParticleVertices[i + 2].Rotation = particles[index].Rotation;
	localParticleVertices[i + 3].Rotation = particles[index].Rotation;
}

void Emitter::Draw(ID3D11DeviceContext* context, Camera* camera)
{
	CopyParticlesToGPU(context);

	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	vs->SetMatrix4x4("view", camera->viewMat);
	vs->SetMatrix4x4("projection", camera->projMat);
	vs->SetShader();
	vs->CopyAllBufferData();

	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();

	if (firstAliveIndex < firstDeadIndex)
	{
		context->DrawIndexed(livingParticleCount * 6, firstAliveIndex * 6, 0);
	}
	else
	{
		context->DrawIndexed(firstDeadIndex * 6, 0, 0);

		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, firstAliveIndex * 6, 0);
	}
}
