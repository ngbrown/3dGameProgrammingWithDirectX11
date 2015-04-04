#pragma once
#include <d3d11.h>

class ShaderHelper
{
public:
	static HRESULT ShaderHelper::LoadCompiledShader(const char *filename, ID3DBlob **blob);
};
