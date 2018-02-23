#include "Game.h"
#include "Vertex.h"

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete lightShader;

	delete cone;
	delete cube;
	delete cylinder;
	delete helix;
	delete sphere;
	delete torus;

	delete particleVS;
	delete particlePS;
	delete emitter;

	delete skyboxPS;
	delete skyboxVS;

	delete combineVS;
	delete combinePS;

	delete pointLightPS;
	delete pointLightVS;
	delete dirLightPS;
	delete dirLightVS;

	delete camera;

	delete stoneMat;

	stoneTexture->Release();
	stoneNormal->Release();
	sampleState->Release();
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthState->Release();
	noBlendState->Release();
	skybox->Release();
	skyboxRasterState->Release();
	skyboxDepthState->Release();
	lightDepthState->Release();
	lightBlendState->Release();
	combineDepthState->Release();

	for (int i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}

	for (int i = 0; i < pLights.size(); i++)
	{
		delete pLights[i];
	}

	for (int i = 0; i < dLights.size(); i++)
	{
		delete dLights[i];
	}
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	CreateWICTextureFromFile(device, context, L"Assets/Textures/StoneWall_albedo.png", 0, &stoneTexture);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/StoneWall_normal.png", 0, &stoneNormal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/particle.jpg", 0, &particleTexture);

	CreateDDSTextureFromFile(device, L"Assets/Textures/skybox.dds", 0, &skybox);

	sampleDesc = { };
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampleDesc, &sampleState);

	stoneMat = new Material(vertexShader, pixelShader, stoneTexture, stoneNormal, skybox, sampleState);

	CreateEntities();

	camera = new Camera((float)width, (float)height);

	CreateLights();

	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC particleDsDesc = {};
	particleDsDesc.DepthEnable = true;
	particleDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	particleDsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&particleDsDesc, &particleDepthState);

	// Blend for particles (additive)
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	// Default Blend
	D3D11_BLEND_DESC blendState = {};
	blendState.RenderTarget[0].BlendEnable = false;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blendState, &noBlendState);

	// Set up particles
	emitter = new Emitter(
		1000,							// Max particles
		20,								// Bursts per second
		5,							// Particles per burst
		0,								// Emitter lifetime
		2,								// Particle lifetime
		2,								// Start size
		0.2f,							// End size
		-3.141593f,						// Minimum Rotation
		3.141593f,						// Maximum Rotation
		-3.141593f,						// Minimum Rotational Velocity
		3.141593f,						// Maximum Rotational Velocity
		XMFLOAT4(1, 0.1f, 0.1f, 1),		// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0.00f),		// End color
		XMFLOAT3(2, 10, 2),				// Max Start velocity
		XMFLOAT3(-2, 5, -2),			// Min Start velocity
		XMFLOAT3(3, 0, 0),				// Start position
		XMFLOAT3(0, -9, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexture);

	// Some states for the sky drawing ----------------------

	// Rasterize state for drawing the "inside"
	D3D11_RASTERIZER_DESC rd = {}; // Remember to zero it out!
	rd.CullMode = D3D11_CULL_FRONT;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.DepthClipEnable = true;
	device->CreateRasterizerState(&rd, &skyboxRasterState);

	// Depth state for accepting pixels with depth EQUAL to existing depth
	D3D11_DEPTH_STENCIL_DESC skyboxDsDesc = {};
	skyboxDsDesc.DepthEnable = true;
	skyboxDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	skyboxDsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyboxDsDesc, &skyboxDepthState);

	// Blend for lights (additive)
	D3D11_BLEND_DESC addBlend = {};
	addBlend.RenderTarget[0].BlendEnable = true;
	addBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	addBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	addBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	addBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	addBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	addBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	addBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&addBlend, &lightBlendState);

	// A depth state for the lights
	D3D11_DEPTH_STENCIL_DESC lightDsDesc = {};
	lightDsDesc.DepthEnable = true;
	lightDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	lightDsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&lightDsDesc, &lightDepthState);

	// A depth state for the combine
	D3D11_DEPTH_STENCIL_DESC combineDsDesc = {};
	particleDsDesc.DepthEnable = false;
	particleDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	particleDsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&combineDsDesc, &combineDepthState);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"DeferredPS.cso");

	lightShader = new SimplePixelShader(device, context);
	lightShader->LoadShaderFile(L"LightsPS.cso");

	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlePS.cso");

	skyboxVS = new SimpleVertexShader(device, context);
	skyboxVS->LoadShaderFile(L"SkyboxVS.cso");

	skyboxPS = new SimplePixelShader(device, context);
	skyboxPS->LoadShaderFile(L"SkyboxPS.cso");

	pointLightPS = new SimplePixelShader(device, context);
	pointLightPS->LoadShaderFile(L"PointLightsPS.cso");

	pointLightVS = new SimpleVertexShader(device, context);
	pointLightVS->LoadShaderFile(L"PointLightsVS.cso");

	dirLightPS = new SimplePixelShader(device, context);
	dirLightPS->LoadShaderFile(L"DirectionalLightsPS.cso");

	dirLightVS = new SimpleVertexShader(device, context);
	dirLightVS->LoadShaderFile(L"DirectionalLightsVS.cso");

	combineVS = new SimpleVertexShader(device, context);
	combineVS->LoadShaderFile(L"CombineVS.cso");

	combinePS = new SimplePixelShader(device, context);
	combinePS->LoadShaderFile(L"CombinePS.cso");
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	cone = new Mesh("cone.obj", device);

	cube = new Mesh("cube.obj", device);

	cylinder = new Mesh("cylinder.obj", device);

	helix = new Mesh("helix.obj", device);

	sphere = new Mesh("sphere.obj", device);

	torus = new Mesh("torus.obj", device);
}

void Game::CreateEntities()
{
	Entity* temp0 = new Entity(cube, stoneMat, worldMatrix, XMFLOAT3(0, -1, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(25, 1, 25));
	entities.push_back(temp0);
	Entity* temp1 = new Entity(cube, stoneMat, worldMatrix, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	entities.push_back(temp1);
	Entity* temp2 = new Entity(sphere, stoneMat, worldMatrix, XMFLOAT3(-3, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	entities.push_back(temp2);
	Entity* temp3 = new Entity(cube, stoneMat, worldMatrix, XMFLOAT3(0, 0, 10), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	entities.push_back(temp3);
}

void Game::CreateLights()
{
	PointLight* temp1 = new PointLight(worldMatrix, sphere, skybox, pointLightVS, pointLightPS, sampleState, XMFLOAT4(+0.1f, +0.1f, +0.1f, +1.0f), XMFLOAT4(+0.4f, +0.4f, +1.0f, +1.0f), XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0, 0, 0), 10.0f);
	pLights.push_back(temp1);

	PointLight* temp2 = new PointLight(worldMatrix, sphere, skybox, pointLightVS, pointLightPS, sampleState, XMFLOAT4(+0.1f, +0.1f, +0.1f, +1.0f), XMFLOAT4(+1.0f, +0.2f, +0.2f, +1.0f), XMFLOAT3(0.0f, 0.0f, 5.0f), XMFLOAT3(0, 0, 0), 7.0f);
	pLights.push_back(temp2);

	DirectionalLight* temp3 = new DirectionalLight(worldMatrix, skybox, dirLightVS, dirLightPS, sampleState, XMFLOAT4(+0.1f, +0.1f, +0.1f, +1.0f), XMFLOAT4(+0.5f, +0.8f, +0.9f, +1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0, -1.0, 1.0));
	dLights.push_back(temp3);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera->GenerateProjection((float)width, (float)height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	entities[1]->rotation.y = totalTime / 2;
	entities[2]->rotation.y = totalTime / 2;

	pLights[1]->SetPosition(sinf(totalTime) * 5.0f, 1.0f, cosf(totalTime) * 5.0f);

	emitter->Update(deltaTime);

	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	GBuffer[0] = colorRTV;
	GBuffer[1] = normalRTV;

	context->OMSetRenderTargets(2, GBuffer, depthStencilView);

	// Background color (Black in this case) for clearing
	const float color[4] = { 0,0,0,0 };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(colorRTV, color);
	context->ClearRenderTargetView(normalRTV, color);
	context->ClearRenderTargetView(lightsRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	context->ClearDepthStencilView(
		lightingDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->Draw(context, camera->projMat, camera->viewMat);
	}

	ClearStates();

	RenderLights();

	ClearStates();

	Combine();

	ClearStates();

	RenderSkybox();

	ClearStates();

	RenderParticles();

	ClearStates();

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::RenderParticles()
{
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
	context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

	emitter->Draw(context, camera);
}

void Game::RenderSkybox()
{
	ID3D11Buffer* skyboxVB = sphere->GetVertexBuffer();
	ID3D11Buffer* skyboxIB = sphere->GetIndexBuffer();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &skyboxVB, &stride, &offset);
	context->IASetIndexBuffer(skyboxIB, DXGI_FORMAT_R32_UINT, 0);

	skyboxVS->SetMatrix4x4("view", camera->viewMat);
	skyboxVS->SetMatrix4x4("projection", camera->projMat);
	skyboxVS->CopyAllBufferData();
	skyboxVS->SetShader();

	skyboxPS->SetShaderResourceView("SkyTexture", skybox);
	skyboxPS->SetSamplerState("BasicSampler", sampleState);
	skyboxPS->CopyAllBufferData();
	skyboxPS->SetShader();

	context->RSSetState(skyboxRasterState);
	context->OMSetDepthStencilState(skyboxDepthState, 0);

	context->DrawIndexed(sphere->GetIndexCount(), 0, 0);
}

void Game::RenderLights()
{
	GBuffer[0] = lightsRTV;
	GBuffer[1] = 0;

	context->OMSetRenderTargets(2, GBuffer, nullptr);

	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(lightBlendState, blend, 0xffffffff);
	context->RSSetState(skyboxRasterState);
	context->OMSetDepthStencilState(lightDepthState, 0);

	for (int i = 0; i < pLights.size(); i++)
	{
		pLights[i]->Draw(context, camera, normalSRV, depthSRV);
	}

	context->RSSetState(0);

	for (int i = 0; i < dLights.size(); i++)
	{
		dLights[i]->Draw(context, camera, normalSRV, depthSRV);
	}
}

void Game::Combine()
{
	GBuffer[0] = backBufferRTV;
	GBuffer[1] = 0;

	context->OMSetRenderTargets(2, GBuffer, depthStencilView);

	context->OMSetDepthStencilState(combineDepthState, 0);

	combineVS->SetShader();

	combinePS->SetShaderResourceView("colorTexture", colorSRV);
	combinePS->SetShaderResourceView("lightTexture", lightsSRV);
	combinePS->SetSamplerState("BasicSampler", sampleState);
	combinePS->CopyAllBufferData();
	combinePS->SetShader();

	context->DrawIndexed(3, 0, 0);
}

void Game::ClearStates()
{
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);

	// Reset states
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetBlendState(noBlendState, 0, 0xffffffff);  // no blending
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	float xChange = x - prevMousePos.x;
	float yChange = y - prevMousePos.y;
	xChange *= 0.002f;
	yChange *= 0.002f;

	camera->MouseRotate(yChange, xChange);

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion