#include "Effects.h"

using namespace DirectX;

BasicEffect* Effects::BasicFX = nullptr;
TexturedEffect* Effects::TexturedFX = nullptr;
TreeSpriteEffect* Effects::TreeSpriteFX = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"BasicVertexShader.cso", L"BasicPixelShader.cso");
	TexturedFX = new TexturedEffect(device, L"TexturedVertexShader.cso", L"TexturedPixelShader.cso");
	TreeSpriteFX = new TreeSpriteEffect(device, L"TreeSpriteVertexShader.cso", L"TreeSpriteGeometryShader.cso", L"TreeSpritePixelShader.cso");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(TexturedFX);
	SafeDelete(TreeSpriteFX);
}
