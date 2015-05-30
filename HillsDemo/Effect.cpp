#include "Effect.h"

#include <ShaderHelper.h>

using namespace DirectX;

Effect::Effect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename)
{
	// Load cso files and create shaders
	HR(ShaderHelper::LoadCompiledShader(vertexShaderFilename, &mVSBlob));
	HR(device->CreateVertexShader(mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), nullptr, &mVertexShader));

	HR(ShaderHelper::LoadCompiledShader(pixelShaderFilename, &mPSBlob));
	HR(device->CreatePixelShader(mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), nullptr, &mPixelShader));
}

Effect::~Effect()
{
}
