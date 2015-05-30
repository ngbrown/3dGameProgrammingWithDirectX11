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
