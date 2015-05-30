#include "Effects.h"

using namespace DirectX;

BasicEffect* Effects::BasicFX = nullptr;
TexturedEffect* Effects::TexturedFX = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"BasicVertexShader.cso", L"BasicPixelShader.cso");
	TexturedFX = new TexturedEffect(device, L"TexturedVertexShader.cso", L"TexturedPixelShader.cso");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(TexturedFX);
}
