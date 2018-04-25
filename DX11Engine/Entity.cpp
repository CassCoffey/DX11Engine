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

void Entity::Draw(ID3D11DeviceContext* context, XMFLOAT4X4 proj, XMFLOAT4X4 view)
{
	GameObject::Draw(context, proj, view);

	material->PrepareMaterial(proj, view, worldMatrix);

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
