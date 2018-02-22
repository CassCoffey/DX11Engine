#include "PointLight.h"

PointLight::PointLight(XMFLOAT4X4 world, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, float iRange) :
	GameObject(world, pos, rot, XMFLOAT3(2 * iRange, 2 * iRange, 2 * iRange))
{
	mesh = iMesh;
	ambientColor = ambient;
	diffuseColor = diffuse;
	position = pos;
	vertexShader = vs;
	pixelShader = ps;
	skybox = sky;
	sampler = sample;
	range = iRange;
}

PointLight::~PointLight()
{

}

void PointLight::PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth)
{
	info = {ambientColor, diffuseColor, position, range};

	vertexShader->SetMatrix4x4("view", camera->viewMat);
	vertexShader->SetMatrix4x4("projection", camera->projMat);
	vertexShader->SetMatrix4x4("world", worldMatrix);

	// Texture Stuff
	pixelShader->SetSamplerState("basicSampler", sampler);
	pixelShader->SetData("light", &info, sizeof(PointLightShaderInfo));
	pixelShader->SetFloat3("CameraPosition", camera->position);
	pixelShader->SetShaderResourceView("normalBuffer", normal);
	pixelShader->SetShaderResourceView("depthBuffer", depth);
	pixelShader->SetShaderResourceView("SkyTexture", skybox);
	pixelShader->SetMatrix4x4("inView", camera->inverseViewMat);
	pixelShader->SetMatrix4x4("inProjection", camera->inverseProjMat);

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
}

void PointLight::Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth)
{
	GameObject::Draw(context, camera->projMat, camera->viewMat);

	PrepareShader(camera, normal, depth);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* vb = mesh->GetVertexBuffer();

	// Draw Mesh
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	context->DrawIndexed(
		mesh->numIndices,     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}
