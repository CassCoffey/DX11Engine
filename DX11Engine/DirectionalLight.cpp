#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(XMFLOAT4X4 world, ID3D11ShaderResourceView* sky, SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11SamplerState* sample, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT3 pos, XMFLOAT3 rot) :
	Light(world, nullptr, sky, vs, ps, sample, ambient, diffuse, pos, rot, XMFLOAT3(1, 1, 1))
{
	
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::PrepareShader(Camera* camera, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* pbr, ID3D11ShaderResourceView* depth)
{
	info = { ambientColor, diffuseColor, rotation, 0.0f };

	// Texture Stuff
	pixelShader->SetSamplerState("basicSampler", sampler);
	pixelShader->SetData("light", &info, sizeof(LightShaderInfo));
	pixelShader->SetFloat3("CameraPosition", camera->position);
	pixelShader->SetShaderResourceView("albedoBuffer", albedo);
	pixelShader->SetShaderResourceView("normalBuffer", normal);
	pixelShader->SetShaderResourceView("depthBuffer", depth);
	pixelShader->SetShaderResourceView("pbrBuffer", pbr);
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

void DirectionalLight::Draw(ID3D11DeviceContext * context, Camera* camera, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* pbr, ID3D11ShaderResourceView* depth)
{
	GameObject::Draw(context, camera->projMat, camera->viewMat);

	PrepareShader(camera, albedo, normal, pbr, depth);

	context->DrawIndexed(3, 0, 0);
}