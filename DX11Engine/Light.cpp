#include "Light.h"

Light::Light(XMFLOAT4X4 world, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale) :
	GameObject(world, pos, rot, scale)
{
	mesh = iMesh;
	ambientColor = ambient;
	diffuseColor = diffuse;
	vertexShader = vs;
	pixelShader = ps;
	skybox = sky;
	sampler = sample;
}

Light::~Light()
{

}

void Light::Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* depth)
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
