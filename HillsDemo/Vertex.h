#pragma once
#include <d3dUtil.h>

namespace Vertex
{
	struct PosNormal
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
	};

	// Basic 32-byte vertex structure.
	struct Basic32
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
	};

	struct TreePointSprite
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT2 Size;
	};
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[2];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* PosNormal;
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* TreePointSprite;
};
