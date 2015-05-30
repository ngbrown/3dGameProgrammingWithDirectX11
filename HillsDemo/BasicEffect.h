#pragma once
#include "Effect.h"

struct cbBasicPerObjectVs
{
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mWorldInvTranspose;
	DirectX::XMFLOAT4X4 mWorldViewProj;
};

struct cbBasicPerObjectPs
{
	Material mMaterial;
};

struct cbBasicPerFramePs
{
	int mLightCount;
	int pad1;
	int pad2;
	int pad3;

	DirectionalLight mDirLights[3];

	DirectX::XMFLOAT3 mEyePosW;
};

class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename);
	~BasicEffect();
	void SetAsEffect(ID3D11DeviceContext* dc);

	void SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, DirectX::CXMMATRIX worldViewProj, DirectX::CXMMATRIX world, DirectX::CXMMATRIX worldInvTranspose);
	void SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const DirectX::XMFLOAT3& eyePosW);
	void SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat);
private:
	ConstantBuffer<cbBasicPerObjectVs> mCbPerObjectVs;
	ConstantBuffer<cbBasicPerObjectPs> mCbPerObjectPs;
	ConstantBuffer<cbBasicPerFramePs> mCbPerFramePs;
};
