#pragma once

#include <d3dUtil.h>
#include "BasicEffect.h"
#include "TexturedEffect.h"
#include "TreeSpriteEffect.h"

class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
	static TexturedEffect* TexturedFX;
	static TreeSpriteEffect* TreeSpriteFX;
};

