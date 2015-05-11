#include "Effects.h"

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

BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename)
	: Effect(device, vertexShaderFilename, pixelShaderFilename)
{
	mCbPerObjectVs.Initialize(device);
	mCbPerObjectPs.Initialize(device);
	mCbPerFramePs.Initialize(device);
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::SetAsEffect(ID3D11DeviceContext* dc)
{
	dc->PSSetShader(mPixelShader, nullptr, 0);
	dc->VSSetShader(mVertexShader, nullptr, 0);
}

void BasicEffect::SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, CXMMATRIX worldViewProj, CXMMATRIX world, CXMMATRIX worldInvTranspose)
{
	// Use a constant buffer. Effect framework deprecated
	cbBasicPerObjectVs perObjectCB;
	XMStoreFloat4x4(&perObjectCB.mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&perObjectCB.mWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&perObjectCB.mWorldInvTranspose, worldInvTranspose);

	mCbPerObjectVs.Data = perObjectCB;
	mCbPerObjectVs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerObjectVs.Buffer();
	dc->VSSetConstantBuffers(0, 1, &buffer);
}

void BasicEffect::SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const XMFLOAT3& eyePosW)
{
	cbBasicPerFramePs perFrameCB;
	perFrameCB.mLightCount = lightCount;
	memcpy(perFrameCB.mDirLights, lights, 3 * sizeof(DirectionalLight));
	perFrameCB.mEyePosW = eyePosW;

	mCbPerFramePs.Data = perFrameCB;
	mCbPerFramePs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerFramePs.Buffer();
	dc->PSSetConstantBuffers(0, 1, &buffer);
}

void BasicEffect::SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat)
{
	cbBasicPerObjectPs perObjectCB;
	perObjectCB.mMaterial = mat;

	mCbPerObjectPs.Data = perObjectCB;
	mCbPerObjectPs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerObjectPs.Buffer();
	dc->PSSetConstantBuffers(1, 1, &buffer);
}

TexturedEffect::TexturedEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename)
	: Effect(device, vertexShaderFilename, pixelShaderFilename)
	, mDevice(device)
{
	mCbPerObjectVs.Initialize(device);
	mCbPerObjectPs.Initialize(device);
	mCbPerFramePs.Initialize(device);
}

TexturedEffect::~TexturedEffect()
{
}

void TexturedEffect::SetAsEffect(ID3D11DeviceContext* dc)
{
	dc->PSSetShader(mPixelShader, nullptr, 0);
	dc->VSSetShader(mVertexShader, nullptr, 0);
}

void TexturedEffect::SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, CXMMATRIX worldViewProj, CXMMATRIX world, CXMMATRIX worldInvTranspose, CXMMATRIX texTransform)
{
	// Use a constant buffer. Effect framework deprecated
	cbTexturedPerObjectVs perObjectCB;
	XMStoreFloat4x4(&perObjectCB.mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&perObjectCB.mWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&perObjectCB.mWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&perObjectCB.mTexTransform, XMMatrixTranspose(texTransform));

	mCbPerObjectVs.Data = perObjectCB;
	mCbPerObjectVs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerObjectVs.Buffer();
	dc->VSSetConstantBuffers(0, 1, &buffer);
}

void TexturedEffect::SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const XMFLOAT3& eyePosW)
{
	cbBasicPerFramePs perFrameCB;
	perFrameCB.mLightCount = lightCount;
	memcpy(perFrameCB.mDirLights, lights, 3 * sizeof(DirectionalLight));
	perFrameCB.mEyePosW = eyePosW;

	mCbPerFramePs.Data = perFrameCB;
	mCbPerFramePs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerFramePs.Buffer();
	dc->PSSetConstantBuffers(0, 1, &buffer);
}

void TexturedEffect::SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat, ID3D11ShaderResourceView* pShaderResourceView)
{
	cbBasicPerObjectPs perObjectCB;
	perObjectCB.mMaterial = mat;
	perObjectCB.mUseTexture = true;

	mCbPerObjectPs.Data = perObjectCB;
	mCbPerObjectPs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerObjectPs.Buffer();
	dc->PSSetConstantBuffers(1, 1, &buffer);
	
	if (pShaderResourceView != nullptr)
	{
		dc->PSSetShaderResources(0, 1, &pShaderResourceView);
	}
}

void TexturedEffect::SetSampler(ID3D11DeviceContext* dc)
{
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ID3D11SamplerState* samplerStates;
	HR(mDevice->CreateSamplerState(&samplerDesc, &samplerStates));
	dc->PSSetSamplers(0, 1, &samplerStates);
}

BasicEffect* Effects::BasicFX = nullptr;
TexturedEffect* Effects::TexturedFX = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"BasicVertexShader.cso", L"BasicPixelShader.cso");
	TexturedFX = new TexturedEffect(device, L"TexturedVertexShader.cso", L"TexturedPixelShader.cso");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(TexturedFX);
}
