#pragma once

#include <d3dUtil.h>
#include <ConstantBuffer.h>

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename);
	virtual ~Effect();
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
};

struct cbTexturedPerObjectVs
{
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mWorldInvTranspose;
	DirectX::XMFLOAT4X4 mWorldViewProj;
	DirectX::XMFLOAT4X4 mTexTransform;
};

struct cbBasicPerObjectPs
{
	Material mMaterial;
	bool mUseTexture;
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

	void SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, DirectX::CXMMATRIX worldViewProj, DirectX::CXMMATRIX world, DirectX::CXMMATRIX worldInvTranspose);
	void SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const DirectX::XMFLOAT3& eyePosW);
	void SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat);
private:
	ConstantBuffer<cbBasicPerObjectVs> mCbPerObjectVs;
	ConstantBuffer<cbBasicPerObjectPs> mCbPerObjectPs;
	ConstantBuffer<cbBasicPerFramePs> mCbPerFramePs;
};

class TexturedEffect : public Effect
{
public:
	TexturedEffect(ID3D11Device* device, const std::wstring& vertexShaderFilename, const std::wstring& pixelShaderFilename);
	~TexturedEffect();
	void SetAsEffect(ID3D11DeviceContext* dc);

	void SetConstantBufferPerObjectVertexShader(ID3D11DeviceContext* dc, DirectX::CXMMATRIX worldViewProj, DirectX::CXMMATRIX world, DirectX::CXMMATRIX worldInvTranspose, DirectX::CXMMATRIX texTransform);
	void SetConstantBufferPerFramePixelShader(ID3D11DeviceContext* dc, const int lightCount, const DirectionalLight* lights, const DirectX::XMFLOAT3& eyePosW);
	void SetConstantBufferPerObjectPixelShader(ID3D11DeviceContext* dc, const Material& mat, ID3D11ShaderResourceView* pShaderResourceView);
	void SetSampler(ID3D11DeviceContext* dc);

private:
	ConstantBuffer<cbTexturedPerObjectVs> mCbPerObjectVs;
	ConstantBuffer<cbBasicPerObjectPs> mCbPerObjectPs;
	ConstantBuffer<cbBasicPerFramePs> mCbPerFramePs;
	ID3D11Device* const mDevice;
};

class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
	static TexturedEffect* TexturedFX;
};

