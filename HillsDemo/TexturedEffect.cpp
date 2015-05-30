#include "TexturedEffect.h"

using namespace DirectX;

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

void TexturedEffect::SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const XMFLOAT3& eyePosW, const XMVECTORF32& fogColor, float fogStart, float fogRange)
{
	cbTexturedPerFramePs perFrameCB;
	perFrameCB.mLightCount = lightCount;
	memcpy(perFrameCB.mDirLights, lights, 3 * sizeof(DirectionalLight));
	perFrameCB.mEyePosW = eyePosW;
	perFrameCB.mFogColor = static_cast<const XMFLOAT4>(fogColor);
	perFrameCB.mFogStart = fogStart;
	perFrameCB.mFogRange = fogRange;


	mCbPerFramePs.Data = perFrameCB;
	mCbPerFramePs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerFramePs.Buffer();
	dc->PSSetConstantBuffers(0, 1, &buffer);
}

void TexturedEffect::SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat, ID3D11ShaderResourceView* pShaderResourceView, bool useTexture, bool useFog)
{
	cbTexturedPerObjectPs perObjectCB;
	perObjectCB.mMaterial = mat;
	perObjectCB.mUseTexture = useTexture;
	perObjectCB.mAlphaClip = true;
	perObjectCB.mFogEnabled = useFog;

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
