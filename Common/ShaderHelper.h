#pragma once
#include <d3d11.h>
#include <string>

class ShaderHelper
{
public:
	static HRESULT ShaderHelper::LoadCompiledShader(const std::wstring& filename, ID3DBlob **blob);
};
