#include "PointLight.h"

PointLight::PointLight(XMFLOAT4X4 world, Mesh* iMesh, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot, float iRange) :
	Light(world, iMesh, sky, vs, ps, sample, ambient, diffuse, pos, rot, XMFLOAT3(2 * iRange, 2 * iRange, 2 * iRange))
{
	range = iRange;
}

PointLight::~PointLight()
{

}

void PointLight::PrepareShader(Camera* camera, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* roughness, ID3D11ShaderResourceView* metal, ID3D11ShaderResourceView* depth)
{
	info = {ambientColor, diffuseColor, position, range};

	vertexShader->SetMatrix4x4("view", camera->viewMat);
	vertexShader->SetMatrix4x4("projection", camera->projMat);
	vertexShader->SetMatrix4x4("world", worldMatrix);

	// Texture Stuff
	pixelShader->SetSamplerState("basicSampler", sampler);
	pixelShader->SetData("light", &info, sizeof(LightShaderInfo));
	pixelShader->SetFloat3("CameraPosition", camera->position);
	pixelShader->SetShaderResourceView("albedoBuffer", albedo);
	pixelShader->SetShaderResourceView("normalBuffer", normal);
	pixelShader->SetShaderResourceView("roughnessBuffer", roughness);
	pixelShader->SetShaderResourceView("metalBuffer", metal);
	pixelShader->SetShaderResourceView("depthBuffer", depth);
	pixelShader->SetShaderResourceView("SkyTexture", skybox);
	pixelShader->SetMatrix4x4("inView", camera->inverseViewMat);
	pixelShader->SetMatrix4x4("inProjection", camera->inverseProjMat);

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