#include "UIManager.h"

UIManager::UIManager(ID3D11Device* device, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11ShaderResourceView* lightTexture)
{
	this->vs = vs;
	this->ps = ps;
	this->lightTexture = lightTexture;
	this->device = device;
}

UIManager::~UIManager()
{
	delete[] localIconVertices;
	vertexBuffer->Release();
	indexBuffer->Release();
}

void UIManager::CopyIconsToGPU(ID3D11DeviceContext * context, std::vector<Icon*> iconList)
{
	RebuildVertsAndIndices(iconList);

	for (int i = 0; i < iconList.size; i++)
			CopyOneIcon(i, iconList[i]);

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localIconVertices, sizeof(IconVertex) * 4 * iconList.size);

	context->Unmap(vertexBuffer, 0);
}

void UIManager::RebuildVertsAndIndices(std::vector<Icon*> iconList)
{
	// Create particle vertices
	localIconVertices = new IconVertex[4 * iconList.size];
	for (int i = 0; i < iconList.size * 4; i += 4)
	{
		localIconVertices[i + 0].UV = XMFLOAT2(0, 0);
		localIconVertices[i + 1].UV = XMFLOAT2(1, 0);
		localIconVertices[i + 2].UV = XMFLOAT2(1, 1);
		localIconVertices[i + 3].UV = XMFLOAT2(0, 1);
	}

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(IconVertex) * 4 * iconList.size;
	device->CreateBuffer(&vbDesc, 0, &vertexBuffer);

	unsigned int* indices = new unsigned int[iconList.size * 6];
	int indexCount = 0;
	for (int i = 0; i < iconList.size * 4; i += 4)
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
	ibDesc.ByteWidth = sizeof(unsigned int) * iconList.size * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	delete[] indices;
}

void UIManager::CopyOneIcon(int index, Icon* icon)
{
	int i = index * 4;

	localIconVertices[i + 0].Position = icon->Position;
	localIconVertices[i + 1].Position = icon->Position;
	localIconVertices[i + 2].Position = icon->Position;
	localIconVertices[i + 3].Position = icon->Position;
		 
	localIconVertices[i + 0].Size = icon->Size;
	localIconVertices[i + 1].Size = icon->Size;
	localIconVertices[i + 2].Size = icon->Size;
	localIconVertices[i + 3].Size = icon->Size;
}

void UIManager::Draw(ID3D11DeviceContext * context, Camera * camera)
{
	CopyIconsToGPU(context, lightIcons);

	UINT stride = sizeof(IconVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	vs->SetMatrix4x4("view", camera->viewMat);
	vs->SetMatrix4x4("projection", camera->projMat);
	vs->SetShader();
	vs->CopyAllBufferData();

	ps->SetShaderResourceView("particle", lightTexture);
	ps->SetShader();
	ps->CopyAllBufferData();

	context->DrawIndexed(lightIcons.size * 6, 0, 0);
}

void UIManager::RegisterLight(PointLight light)
{
	Icon* temp = {};
	temp->Position = light.Position;
	temp->Size = 1;

	lightIcons.push_back(temp);
}
