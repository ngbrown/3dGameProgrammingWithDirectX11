#pragma once

#include <d3dUtil.h>
#include <ConstantBuffer.h>

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename);
	~Effect();
	void virtual SetAsEffect(ID3D11DeviceContext* dc) = 0;

	ID3DBlob* mVSBlob;
private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

	ID3DBlob* mPSBlob;

protected:
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;
};

struct cbBasicPerObjectVs
{
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mWorldInvTranspose;
	DirectX::XMFLOAT4X4 mWorldViewProj;
	DirectX::XMFLOAT4X4 mTexTransform;
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

	float mFogStart;
	float mFogRange;
	DirectX::XMFLOAT4 mFogColor;
};

class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename);
	~BasicEffect();
	void SetAsEffect(ID3D11DeviceContext* dc);

	void SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, const DirectX::CXMMATRIX& worldViewProj, const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& worldInvTranspose);
	void SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const DirectX::XMFLOAT3& eyePosW);
	void SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat);
private:
	ConstantBuffer<cbBasicPerObjectVs> mCbPerObjectVs;
	ConstantBuffer<cbBasicPerObjectPs> mCbPerObjectPs;
	ConstantBuffer<cbBasicPerFramePs> mCbPerFramePs;
};


class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
};

