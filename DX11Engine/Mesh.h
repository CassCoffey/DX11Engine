#pragma once

#include "Vertex.h"
#include <fstream>
#include <vector>
#include <d3d11.h>

// For the DirectX Math library
using namespace DirectX;

class Mesh
{
public:
	Mesh(Vertex* vertices, int vertNum, UINT* indices, int indexNum, ID3D11Device* device);
	Mesh(char* file, ID3D11Device* device);
	~Mesh();

	void SetBuffers(Vertex* vertices, UINT* indices, int vertNum, int indexNum, ID3D11Device* device);

	void CalculateTangents(Vertex* vertices, int vertNum, UINT* indices, int indexNum);

	ID3D11Buffer* const* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	int numIndices;
};

