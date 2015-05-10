#include "Vertex.h"
#include "Effects.h"

using namespace DirectX;

// Create the vertex input layout.
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormal[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

ID3D11InputLayout* InputLayouts::PosNormal = nullptr;

void InputLayouts::InitAll(ID3D11Device* device)
{
	ID3DBlob* mVSBlob = Effects::BasicFX->mVSBlob;
	// Create the input layout
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormal, 2, mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(), &PosNormal));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(PosNormal);
}
