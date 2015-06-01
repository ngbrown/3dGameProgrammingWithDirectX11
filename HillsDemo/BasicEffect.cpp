#include "BasicEffect.h"

using namespace DirectX;

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
	dc->GSSetShader(nullptr, nullptr, 0);
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
