#pragma once

#include "DXCore.h"
#include "DDSTextureLoader.h"
#include "Mesh.h"
#include "Entity.h"
#include "Emitter.h"
#include "SimpleShader.h"
#include "Camera.h"
#include "PointLight.h"
#include "WICTextureLoader.h"
#include <DirectXMath.h>

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void RenderParticles();
	void RenderSkybox();
	void RenderLights();
	void Combine();
	void ClearStates();

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Camera
	Camera* camera;

	// Texture
	ID3D11ShaderResourceView* stoneTexture;
	ID3D11ShaderResourceView* stoneNormal;

	// Skybox
	ID3D11ShaderResourceView* skybox;
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;

	ID3D11RasterizerState* skyboxRasterState;
	ID3D11DepthStencilState* skyboxDepthState;

	// Particle stuff
	ID3D11ShaderResourceView* particleTexture;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	ID3D11DepthStencilState* particleDepthState;
	ID3D11BlendState* particleBlendState;
	ID3D11BlendState* noBlendState;
	Emitter* emitter;

	// Sampler State
	ID3D11SamplerState* sampleState;
	D3D11_SAMPLER_DESC sampleDesc;

	// Materials
	Material* stoneMat;

	// Meshes
	Mesh* cone;
	Mesh* cube;
	Mesh* cylinder;
	Mesh* helix;
	Mesh* sphere;
	Mesh* torus;

	// Lights
	SimplePixelShader* lightPS;
	std::vector<PointLight*> pLights;
	ID3D11DepthStencilState* lightDepthState;

	// Entities
	std::vector<Entity*> entities;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();
	void CreateEntities();
	void CreateLights();

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	SimplePixelShader* lightShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};

