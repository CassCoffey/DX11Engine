#include "Material.h"



Material::Material(SimpleVertexShader * vs, SimplePixelShader * ps, ID3D11ShaderResourceView* tex, ID3D11ShaderResourceView* norm, ID3D11SamplerState* ss)
{
	pixelShader = ps;
	vertexShader = vs;
	texture = tex;
	normals = norm;
	sampleState = ss;
}

Material::~Material()
{
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}

ID3D11ShaderResourceView * Material::getTexture()
{
	return texture;
}

ID3D11ShaderResourceView * Material::getNormals()
{
	return normals;
}

ID3D11SamplerState * Material::getSampleState()
{
	return sampleState;
}
