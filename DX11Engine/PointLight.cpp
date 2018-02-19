#include "PointLight.h"

PointLight::PointLight(XMFLOAT4X4 world, Mesh* iMesh, SimpleVertexShader* vs, SimplePixelShader* ps, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale) :
	GameObject(world, pos, rot, scale)
{
	mesh = iMesh;
	ambientColor = ambient;
	diffuseColor = diffuse;
	position = pos;
	vertexShader = vs;
	pixelShader = ps;
}

PointLight::~PointLight()
{

}

void PointLight::PrepareShader(Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* worldPos)
{
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	vertexShader->SetMatrix4x4("view", camera->viewMat);
	vertexShader->SetMatrix4x4("projection", camera->projMat);
	vertexShader->SetMatrix4x4("world", worldMatrix);

	// Texture Stuff

	pixelShader->SetFloat3("CameraPosition", camera->position);
	pixelShader->SetShaderResourceView("NormalBuffer", normal);
	pixelShader->SetShaderResourceView("WorldPosBuffer", worldPos);

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

void PointLight::Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* worldPos)
{
	GameObject::Draw(context, camera->projMat, camera->viewMat);

	PrepareShader(camera, normal, worldPos);

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
