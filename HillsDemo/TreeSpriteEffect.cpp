#include "TreeSpriteEffect.h"

using namespace DirectX;

static cbTreeSpriteFixedGs fixedGs = 
{
	{
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f }
	}
};

TreeSpriteEffect::TreeSpriteEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& geometryShaderFilename, const std::wstring& pixelShaderFilename)
	: Effect(device, vertexShaderFilename, geometryShaderFilename, pixelShaderFilename)
	, mDevice(device)
{
	mCbPerObjectGs.Initialize(device);
	mCbPerFrameGs.Initialize(device);
	mCbFixedGs.Initialize(device);
	mCbPerObjectPs.Initialize(device);
	mCbPerFramePs.Initialize(device);
}

TreeSpriteEffect::~TreeSpriteEffect()
{
}

void TreeSpriteEffect::SetAsEffect(ID3D11DeviceContext* dc)
{
	dc->PSSetShader(mPixelShader, nullptr, 0);
	dc->GSSetShader(mGeometryShader, nullptr, 0);
	dc->VSSetShader(mVertexShader, nullptr, 0);

	SetConstantBufferFixedGeometryShader(dc);
}

void TreeSpriteEffect::SetConstantBufferPerFrameGeometryShader(ID3D11DeviceContext* dc, const XMFLOAT3& eyePosW)
{
	cbTreeSpritePerFrameGs perFrameCB;
	perFrameCB.mEyePosW = eyePosW;

	mCbPerFrameGs.Data = perFrameCB;
	mCbPerFrameGs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerFrameGs.Buffer();
	dc->GSSetConstantBuffers(0, 1, &buffer);
}

void TreeSpriteEffect::SetConstantBufferPerObjectGeometryShader(ID3D11DeviceContext* dc, CXMMATRIX viewProj)
{
	// Use a constant buffer. Effect framework deprecated
	cbTreeSpritePerObjectGs perObjectCB;
	XMStoreFloat4x4(&perObjectCB.mViewProj, XMMatrixTranspose(viewProj));

	mCbPerObjectGs.Data = perObjectCB;
	mCbPerObjectGs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbPerObjectGs.Buffer();
	dc->GSSetConstantBuffers(1, 1, &buffer);
}

void TreeSpriteEffect::SetConstantBufferFixedGeometryShader(ID3D11DeviceContext* dc)
{
	mCbFixedGs.Data = fixedGs;
	mCbFixedGs.ApplyChanges(dc);
	ID3D11Buffer* buffer = mCbFixedGs.Buffer();
	dc->GSSetConstantBuffers(2, 1, &buffer);
}

void TreeSpriteEffect::SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const XMFLOAT3& eyePosW, const XMVECTORF32& fogColor, float fogStart, float fogRange)
{
	cbTreeSpritePerFramePs perFrameCB;
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

void TreeSpriteEffect::SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat, ID3D11ShaderResourceView* pShaderResourceView, bool useTexture, bool useFog)
{
	cbTreeSpritePerObjectPs perObjectCB;
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

void TreeSpriteEffect::SetSampler(ID3D11DeviceContext* dc)
{
	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ID3D11SamplerState* samplerStates;
	HR(mDevice->CreateSamplerState(&samplerDesc, &samplerStates));
	dc->PSSetSamplers(0, 1, &samplerStates);
}
