#pragma once
#include "Effect.h"

struct cbTreeSpritePerObjectGs
{
	DirectX::XMFLOAT4X4 mViewProj;
};

struct cbTreeSpritePerFrameGs
{
	DirectX::XMFLOAT3 mEyePosW;
};

struct cbTreeSpriteFixedGs
{
	DirectX::XMFLOAT4 mTexC[4];
};

struct cbTreeSpritePerObjectPs
{
	Material mMaterial;
	int mUseTexture;
	int mAlphaClip;
	int mFogEnabled;
};

struct cbTreeSpritePerFramePs
{
	int mLightCount;
	int pad1;
	int pad2;
	int pad3;

	DirectionalLight mDirLights[3];

	DirectX::XMFLOAT3 mEyePosW;

	float mFogStart;
	float mFogRange;
	char pad4[12];
	DirectX::XMFLOAT4 mFogColor;
};

class TreeSpriteEffect : public Effect
{
public:
	TreeSpriteEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& geometryShaderFilename, const std::wstring& pixelShaderFilename);
	~TreeSpriteEffect();
	void SetAsEffect(ID3D11DeviceContext* dc);

	void SetConstantBufferPerFrameGeometryShader(ID3D11DeviceContext* dc, const DirectX::XMFLOAT3& eyePosW);
	void SetConstantBufferPerObjectGeometryShader(ID3D11DeviceContext* dc, DirectX::CXMMATRIX viewProj);
	void SetConstantBufferFixedGeometryShader(ID3D11DeviceContext* dc);
	void SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const DirectX::XMFLOAT3& eyePosW, const DirectX::XMVECTORF32& fogColor = DirectX::Colors::Silver, float fogStart = 200.0f, float fogRange = 800.0f);
	void SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat, ID3D11ShaderResourceView* pShaderResourceView, bool useTexture = true, bool useFog = true);
	void SetSampler(ID3D11DeviceContext* dc);

private:
	ConstantBuffer<cbTreeSpritePerObjectGs> mCbPerObjectGs;
	ConstantBuffer<cbTreeSpritePerFrameGs> mCbPerFrameGs;
	ConstantBuffer<cbTreeSpriteFixedGs> mCbFixedGs;
	ConstantBuffer<cbTreeSpritePerObjectPs> mCbPerObjectPs;
	ConstantBuffer<cbTreeSpritePerFramePs> mCbPerFramePs;
	ID3D11Device* const mDevice;
};
