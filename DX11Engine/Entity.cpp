#include "Entity.h"


Entity::Entity(Mesh * iMesh, Material* iMaterial, XMFLOAT4X4 iWorld, XMFLOAT3 iPos, XMFLOAT3 iRot, XMFLOAT3 iScale) :
	GameObject(iWorld, iPos, iRot, iScale)
{
	mesh = iMesh;
	material = iMaterial;
}

Entity::~Entity()
{

}

void Entity::PrepareMaterial(XMFLOAT4X4 proj, XMFLOAT4X4 view)
{
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	material->GetVertexShader()->SetMatrix4x4("view", view);
	material->GetVertexShader()->SetMatrix4x4("projection", proj);
	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);

	// Texture Stuff
	material->GetPixelShader()->SetSamplerState("basicSampler", material->getSampleState());
	material->GetPixelShader()->SetShaderResourceView("diffuseTexture", material->getTexture());
	material->GetPixelShader()->SetShaderResourceView("normalTexture", material->getNormals());

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	material->GetVertexShader()->CopyAllBufferData();
	material->GetPixelShader()->CopyAllBufferData();
}

void Entity::Draw(ID3D11DeviceContext* context, XMFLOAT4X4 proj, XMFLOAT4X4 view)
{
	GameObject::Draw(context, proj, view);

	PrepareMaterial(proj, view);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Draw Mesh
	context->IASetVertexBuffers(0, 1, mesh->GetVertexBuffer(), &stride, &offset);
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
