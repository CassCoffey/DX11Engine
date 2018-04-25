#include "Material.h"



Material::Material(SimpleVertexShader* vs, SimplePixelShader* ps, XMFLOAT4 col, XMFLOAT2 scale, ID3D11ShaderResourceView* tex, ID3D11ShaderResourceView* norm, ID3D11ShaderResourceView* rough, ID3D11ShaderResourceView* met, ID3D11ShaderResourceView* sky, ID3D11SamplerState* ss)
{
	pixelShader = ps;
	vertexShader = vs;
	texture = tex;
	normals = norm;
	metal = met;
	roughness = rough;
	color = col;
	uvScale = scale;
	skybox = sky;
	sampleState = ss;
}

Material::~Material()
{
}

void Material::PrepareMaterial(XMFLOAT4X4 proj, XMFLOAT4X4 view, XMFLOAT4X4 world)
{
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	vertexShader->SetMatrix4x4("view", view);
	vertexShader->SetMatrix4x4("projection", proj);
	vertexShader->SetMatrix4x4("world", world);
	vertexShader->SetFloat2("uvScale", uvScale);

	pixelShader->SetFloat4("Color", color);
	pixelShader->CopyBufferData("perMaterial");

	// Texture Stuff
	pixelShader->SetSamplerState("basicSampler", sampleState);
	pixelShader->SetShaderResourceView("diffuseTexture", texture);
	pixelShader->SetShaderResourceView("normalTexture", normals);
	pixelShader->SetShaderResourceView("roughnessTexture", roughness);
	pixelShader->SetShaderResourceView("metalTexture", metal);
	pixelShader->SetShaderResourceView("skyTexture", skybox);

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
