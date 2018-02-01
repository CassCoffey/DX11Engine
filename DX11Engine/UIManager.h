#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Lights.h"
#include "Camera.h"
#include "SimpleShader.h"

struct Icon
{
	DirectX::XMFLOAT3 Position;
	float Size;
};

struct IconVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	float Size;
};

class UIManager
{
public:
	UIManager(ID3D11Device* device,	SimpleVertexShader* vs,	SimplePixelShader* ps, ID3D11ShaderResourceView* lightTexture);
	~UIManager();

	void CopyIconsToGPU(ID3D11DeviceContext* context, std::vector<Icon*> iconList);
	void CopyOneIcon(int index, Icon* icon);
	void Draw(ID3D11DeviceContext* context, Camera* camera);

	void RegisterLight(PointLight light);

private:

	void RebuildVertsAndIndices(std::vector<Icon*> iconList);

	std::vector<Icon*> lightIcons;

	// Rendering
	IconVertex* localIconVertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	SimpleVertexShader* vs;
	SimplePixelShader* ps;
	ID3D11Device* device;

	ID3D11ShaderResourceView* lightTexture;
};

