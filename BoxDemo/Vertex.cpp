#include "Vertex.h"
#include "Effects.h"

using namespace DirectX;

// Create the vertex input layout.
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormal[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

ID3D11InputLayout* InputLayouts::PosNormal = nullptr;
ID3D11InputLayout* InputLayouts::Basic32 = nullptr;

void InputLayouts::InitAll(ID3D11Device* device)
{
	// Create the input layout
	ID3DBlob* mVSBlob = Effects::BasicFX->mVSBlob;
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormal, 2, mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(), &PosNormal));

	mVSBlob = Effects::TexturedFX->mVSBlob;
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(), &Basic32));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(PosNormal);
	ReleaseCOM(Basic32);
}
